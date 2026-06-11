#include <wsrv/server.hpp>
#include <wsrv/request_handler.hpp>
#include <wsrv/session.hpp>
#include <wsrv/exceptions.hpp>
#include <wsrv/sqlite3_session_manager.hpp>
#include <wsrv/middleware.hpp>
#include <wsrv/application.hpp>
#include <wsrv/middleware/locale.hpp>
#include <wsrv/middleware/request_logger.hpp>
#include <wsrv/auth/authenticator.hpp>
#include <mutex>
#include <iostream>

#include <twig/renderer.hpp>
#include <twig/translator.hpp>

#include "routes.hpp"
#include "util/gpx.hpp"
#include "connection_pool.hpp"
#include "context.hpp"

using namespace ws ;
using namespace std ;



const char *web_root = "/home/malasiot/source/wsrv/app/web/" ;

void render(const char *templ, twig::TemplateRenderer &rdr, 
    HTTPServerRequest &req, HTTPServerResponse &res, const Variant::Object &data = {}) {
    auto user_data = req.data().get<IAuthenticatedUser>() ;
    auto locale = req.data().get<LocaleResolverData>() ;
    rdr.setLocale(locale->locale()) ;

    Variant::Object ctx{
                    {"app", Variant::Object{
                        {
                            "locale", locale->locale()
                        }, {
                            "is_authenticated", user_data != nullptr
                        }}
                    },
                };

    ctx.insert(data.begin(), data.end()) ;
       
    res.write(rdr.render(templ, ctx ));
}

int main(int argc, char *argv[]) {

    HttpServer server("127.0.0.1:5110") ;

    AppContext app_context(Variant::fromJSONFile("/Users/malasiot/wsrv/app/config.json"));

    Application app ;
    
    server.setHandler(&app) ;

    auto locale = std::make_shared<LocaleResolver>(app_context.locales_, app_context.default_locale_) ;

    auto login = std::make_shared<SessionLoginController>(app_context.session_manager_.get(), app_context.auth_.get());
    auto auth = std::make_shared<SessionRequireAuth>(app_context.session_manager_.get(), app_context.auth_.get());
    auto check = std::make_shared<SessionCheckAuth>(app_context.session_manager_.get(), app_context.auth_.get()) ;
    auto logout = std::make_shared<SessionLogoutController>(app_context.session_manager_.get()) ;

    DebugLogger logger ;
    app.useGlobal(std::make_shared<RequestLogger>(logger)) ;

    string url = app_context.url_ ;//"mysql:db=hp;host=localhost;username=malasiot;password=sotiris99" ;
    ConnectionPool con(url, app_context.num_connections_) ;
    
    {
        xdb::Connection  c(url) ;
        Routes::createTables(c) ;
    }

    Blueprint admin("admin/") ;

    

    admin.addRoute("GET|POST", "login/", [&](HTTPServerRequest& req, HTTPServerResponse& resp) {
        auto user_data = req.data().get<IAuthenticatedUser>() ;
        auto locale = req.data().get<LocaleResolverData>() ;
        app_context.rdr_->setLocale(locale->locale()) ;

         if ( req.getMethod() == "GET" ) {
            render("admin/login.html", *app_context.rdr_, req, resp) ;
        } else {
            if ( user_data )
                resp.json(R"({"success": true })");
            else
                resp.json(R"({"success": false, "message": "Invalid password or username credentials."})");
        }
        
    }, {locale, login} ) ;

    admin.addRoute("GET", "dashboard/", [&](HTTPServerRequest& req, HTTPServerResponse& resp) {
        render("admin/dashboard.html", *app_context.rdr_, req, resp) ;
        
    }, {locale, auth} ) ;

    admin.addRoute("GET", "upload/", [&](HTTPServerRequest& req, HTTPServerResponse& resp) {
            render("admin/upload.html", *app_context.rdr_, req, resp) ;
    }, {locale, auth} ) ;

    admin.addRoute("GET", "logout/", [&](HTTPServerRequest& req, HTTPServerResponse& resp) {
         auto user_data = req.data().get<IAuthenticatedUser>() ;
           if ( !user_data )
                resp.json(R"({"success": true })");
    }, {logout} ) ;


    Blueprint routes("routes/") ;

    routes.addRoute("POST", "create/", [&](HTTPServerRequest& req, HTTPServerResponse& resp) {
        auto locale = req.data().get<LocaleResolverData>() ;
        GPX data ;
        auto files = req.getUploadedFiles();
        if ( files.count("gps_file") ) {
            istringstream strm(files["gps_file"].data_) ;
           
            GPXParser parser ;
            if ( !parser.parse(strm, data) ) {
                 string error = app_context.translator_->translate(twig::tr("gpx.error"), locale->locale()) ;
                 resp.json("{\"error\":\"" + error + "\"}") ;
                 return ;
            }
        }

     //   const char *spatialite = "/opt/homebrew/lib/mod_spatialite";
    //    xdb::Connection con(std::string("sqlite:mode=rc;db=") + web_root + "db/db.sqlite" + ";ext=" + spatialite);
    
    ConnectionPool::ConnectionPtr conn = con.acquireConnection();
 Variant::Object title{{locale->locale(), req.getPostAttribute("title")}};
    
        uint64_t id = Routes::createRoute(*conn, title, req.getPostAttribute("difficulty"), data) ;


       
     
        resp.json("{\"id\":" + std::to_string(id) + "}") ;
    }, {locale, auth} ) ;

    routes.addRoute("GET", "edit/{id}/", [&](HTTPServerRequest& req, HTTPServerResponse& resp) {
        render("routes/edit.html", *app_context.rdr_, req, resp, Variant::Object{ 
            {"route", Variant::Object{
                { "id", "10"},
                { "title", "kkk" }
            }
            }}) ;
    }, {locale, auth} ) ;

    app.addRoute("GET", "/", [&](HTTPServerRequest& req, HTTPServerResponse& resp) {
        resp.redirect("/home/");
    } ) ;

    app.addRoute("GET", "home/", [&](HTTPServerRequest& req, HTTPServerResponse& resp) {
        render("landing.html", *app_context.rdr_, req, resp) ;
    }, { locale, check } ) ;

    app.addRoute("GET", "routes/", [&](HTTPServerRequest& req, HTTPServerResponse& resp) {
         render("routes.html", *app_context.rdr_, req, resp) ;
    }, { locale, check } ) ;
    
    app.addRoute("GET", "public/{file:.*}", [&app_context](HTTPServerRequest& req, HTTPServerResponse& resp) {
     //   resp.write(rdr.renderString("Requested file: {{file}}", { {"file", req.getRouteAttribute("file")} })) ;
      
        if ( !resp.serveStaticFile(string(web_root) + "public/", req.getRouteAttribute("file")) ) {
             resp = HTTPServerResponse::stockReply(HTTPServerResponse::not_found) ;
        }
    }) ;

    app.registerBlueprint(admin) ;
    app.registerBlueprint(routes) ;

    
    server.run() ;
}
