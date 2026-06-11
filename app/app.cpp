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

using namespace ws ;
using namespace std ;

class User: public IAuthenticatedUser {
public:

    User(const std::string &id): id_(id) {} 
    virtual std::string getUniqueId() const override { return id_ ; };
private:
    std::string id_ ;
};

class SimpleAuthProvider: public IAuthenticationProvider {
 virtual std::shared_ptr<IAuthenticatedUser> authenticate(
        const std::string& username, 
        const std::string& password
    ) override {
        
        if ( username == "root" && password == "test" ) return make_shared<User>("1");
        else return nullptr ;
    }

    std::shared_ptr<IAuthenticatedUser> loadUser(const std::string &id) override {
        return make_shared<User>(id) ;
    }
};

const char *web_root = "/Users/malasiot/source/wsrv/app/web/" ;

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

    Application app ;
    
    server.setHandler(&app) ;

    SQLite3SessionManager session_manager("/tmp/session.sqlite");

    std::shared_ptr<twig::FileSystemTemplateLoader> loader(new twig::FileSystemTemplateLoader({std::string(web_root) + "/templates/"}));
    twig::TemplateRenderer rdr(loader) ;

    twig::TranslationManager translator ;
    translator.loadAllFromDirectory(std::string(web_root) + "/translations/");
    rdr.setTranslationManager(&translator);

    auto locale = std::make_shared<LocaleResolver>(std::vector<std::string>{"en", "el"}, "en") ;

    SimpleAuthProvider provider ;
    auto login = std::make_shared<SessionLoginController>(&session_manager, &provider);
    auto auth = std::make_shared<SessionRequireAuth>(&session_manager, &provider);
    auto check = std::make_shared<SessionCheckAuth>(&session_manager, &provider) ;
    auto logout = std::make_shared<SessionLogoutController>(&session_manager) ;

    DebugLogger logger ;
    app.useGlobal(std::make_shared<RequestLogger>(logger)) ;

    string url = "mysql:db=hp;host=localhost;username=malasiot;password=sotiris99" ;
    ConnectionPool con(url, 4) ;
    
    {
        xdb::Connection  c(url) ;
        Routes::initTables(c) ;
    }

    Blueprint admin("admin/") ;

    admin.addRoute("GET|POST", "login/", [&](HTTPServerRequest& req, HTTPServerResponse& resp) {
        auto user_data = req.data().get<IAuthenticatedUser>() ;
        auto locale = req.data().get<LocaleResolverData>() ;
        rdr.setLocale(locale->locale()) ;

         if ( req.getMethod() == "GET" ) {
            render("admin/login.html", rdr, req, resp) ;
        } else {
            if ( user_data )
                resp.json(R"({"success": true })");
            else
                resp.json(R"({"success": false, "message": "Invalid password or username credentials."})");
        }
        
    }, {locale, login} ) ;

    admin.addRoute("GET", "dashboard/", [&](HTTPServerRequest& req, HTTPServerResponse& resp) {
        render("admin/dashboard.html", rdr, req, resp) ;
        
    }, {locale, auth} ) ;

    admin.addRoute("GET", "upload/", [&](HTTPServerRequest& req, HTTPServerResponse& resp) {
            render("admin/upload.html", rdr, req, resp) ;
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
                 string error = translator.translate(twig::tr("gpx.error"), locale->locale()) ;
                 resp.json("{\"error\":\"" + error + "\"}") ;
                 return ;
            }
        }
     //   const char *spatialite = "/opt/homebrew/lib/mod_spatialite";
    //    xdb::Connection con(std::string("sqlite:mode=rc;db=") + web_root + "db/db.sqlite" + ";ext=" + spatialite);
    
    ConnectionPool::ConnectionPtr conn = con.acquireConnection();

    
        uint64_t id = Routes::createRoute(*conn, req.getPostAttribute("title"), req.getPostAttribute("difficulty"), data) ;
        resp.json("{\"id\":" + std::to_string(id) + "}") ;
    }, {locale, auth} ) ;

    routes.addRoute("GET", "edit/{id}/", [&](HTTPServerRequest& req, HTTPServerResponse& resp) {
        render("routes/edit.html", rdr, req, resp, Variant::Object{ 
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
        render("landing.html", rdr, req, resp) ;
    }, { locale, check } ) ;

    app.addRoute("GET", "routes/", [&](HTTPServerRequest& req, HTTPServerResponse& resp) {
         render("routes.html", rdr, req, resp) ;
    }, { locale, check } ) ;
    
    app.addRoute("GET", "public/{file:.*}", [&session_manager, &rdr](HTTPServerRequest& req, HTTPServerResponse& resp) {
     //   resp.write(rdr.renderString("Requested file: {{file}}", { {"file", req.getRouteAttribute("file")} })) ;
      
        if ( !resp.serveStaticFile(string(web_root) + "public/", req.getRouteAttribute("file")) ) {
             resp = HTTPServerResponse::stockReply(HTTPServerResponse::not_found) ;
        }
    }) ;

    app.registerBlueprint(admin) ;
    app.registerBlueprint(routes) ;

    
    server.run() ;
}
