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
#include "util/string_util.hpp"

using namespace ws ;
using namespace std ;


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

    AppContext app_context(Variant::fromJSONFile("/home/malasiot/source/wsrv/app/config.json"));

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


    Blueprint api("api/") ;

    api.addRoute("POST", "/photos/upload/{id}/", [&](HTTPServerRequest& req, HTTPServerResponse& resp) {
        
       
        int64_t track_id = stoull(req.getRouteAttribute("id")) ;
        int64_t wpt_id = -1 ;
        string wpt_id_str = req.getPostAttribute("wpt") ;
        if ( !wpt_id_str.empty() )
            wpt_id = stoll(wpt_id_str) ;
        auto files = req.getUploadedFiles();
        for( const auto &pic: files ) {
            if ( pic.id_ != "image") continue ;
            string mime = mimeFromHeader(pic.data_) ;

            ConnectionPool::ConnectionPtr conn = con.acquireConnection();
            uint64_t photo_id = Routes::addPhoto(*conn, track_id, pic.data_, mime, wpt_id) ;
            string rel_url =  "/api/photo/" + std::to_string(photo_id) + "/";

            Variant::Object result{{"success", true}, {"id", photo_id }, {"url", rel_url}}; 
            resp.json(Variant(result).toJSON()) ; 
            return ;
        }
        Variant::Object result{{"success", false}} ;
        resp.json(Variant(result).toJSON()) ;
    });

     api.addRoute("GET", "/photo/{id}/",  [&](HTTPServerRequest& req, HTTPServerResponse& resp) {
        uint64_t id = stoull(req.getRouteAttribute("id")) ;

        ConnectionPool::ConnectionPtr conn = con.acquireConnection();
        auto photo = Routes::getPhoto(*conn, id) ;
        if ( photo ) 
            resp.write(photo->data_, photo->mime_); 
     }) ;

    api.addRoute("POST", "/photos/delete/{id}/",  [&](HTTPServerRequest& req, HTTPServerResponse& resp) {
        int64_t photo_id = stoll(req.getRouteAttribute("id")) ;

        ConnectionPool::ConnectionPtr conn = con.acquireConnection();
        Routes::deletePhoto(*conn, photo_id) ;
        Variant::Object result{{"success", true}} ;
        resp.json(Variant(result).toJSON()) ;
        
     }) ;

      api.addRoute("POST", "/photos/caption/{id}/",  [&](HTTPServerRequest& req, HTTPServerResponse& resp) {
       auto locale = req.data().get<LocaleResolverData>() ;
        int64_t photo_id = stoll(req.getRouteAttribute("id")) ;
        string caption = req.getPostAttribute("caption");

        ConnectionPool::ConnectionPtr conn = con.acquireConnection();
        Routes::updatePhotoCaption(*conn, photo_id, caption, locale->locale()) ;
        Variant::Object result{{"success", true}} ;
        resp.json(Variant(result).toJSON()) ;
        
     }, {locale}) ;



    Blueprint routes("routes/") ;

    routes.addRoute("POST", "create/", [&](HTTPServerRequest& req, HTTPServerResponse& resp) {
        auto locale = req.data().get<LocaleResolverData>() ;
        GPX data ;
        auto files = req.getUploadedFiles();
        for ( const auto &file: files ) {
            if ( file.id_ != "gps_file" ) continue ;
            istringstream strm(file.data_) ;
           
            GPXParser parser ;
            if ( !parser.parse(strm, data) ) {
                 string error = app_context.translator_->translate(twig::tr("gpx.error"), locale->locale()) ;
                 resp.json("{\"error\":\"" + error + "\"}") ;
                 return ;
            }
        }

        ConnectionPool::ConnectionPtr conn = con.acquireConnection();

        Variant::Object title{{locale->locale(), req.getPostAttribute("title")}};
    
        int64_t id = Routes::createRoute(*conn, title, req.getPostAttribute("difficulty"), data) ;
   
        resp.json("{\"id\":" + std::to_string(id) + "}") ;
    }, {locale, auth} ) ;

    routes.addRoute("GET", "edit/{id}/", [&](HTTPServerRequest& req, HTTPServerResponse& resp) {
        auto locale = req.data().get<LocaleResolverData>() ;
         ConnectionPool::ConnectionPtr conn = con.acquireConnection();

        string id = req.getRouteAttribute("id") ;
        auto photos = Routes::getAllPhotosForTrack(*conn, std::stoll(id), locale->locale()) ;
        Variant::Array photo_list ;
        for( const auto &photo: photos) {
            Variant::Object item{{"id", photo.id_}, {"caption", photo.caption_}, {"url", "/api/photo/" + std::to_string(photo.id_)}} ;
            photo_list.emplace_back(std::move(item));
        }
        render("routes/edit.html", *app_context.rdr_, req, resp, Variant::Object{ 
            {"route", Variant::Object{
                { "id", req.getRouteAttribute("id")},
                { "title", "kkk" },
                { "photos", photo_list}
            }
            }}) ;
    }, {locale, auth} ) ;


    api.addRoute("POST", "/routes/update/{id}/", [&](HTTPServerRequest& req, HTTPServerResponse& resp) {
        auto locale = req.data().get<LocaleResolverData>() ;
         ConnectionPool::ConnectionPtr conn = con.acquireConnection();

         try {
        string id = req.getRouteAttribute("id") ;

        string title = req.getPostAttribute("title") ;
        string desc = req.getPostAttribute("desc") ;
        string duration = req.getPostAttribute("duration") ;
        string length = req.getPostAttribute("length") ;
        string difficulty = req.getPostAttribute("difficulty") ;

        Routes::updateRoute(*conn, stoll(id), title, desc, stof(duration), stof(length), difficulty, locale->locale() );
        Variant::Object result{{"success", true}} ;
        resp.json(Variant(result).toJSON()) ;
         } catch ( invalid_argument &e ) {
            Variant::Object result{{"success", false}} ;
        resp.json(Variant(result).toJSON()) ;
         }
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
        if ( !resp.serveStaticFile(app_context.web_root_ + "public/", req.getRouteAttribute("file")) ) {
             resp = HTTPServerResponse::stockReply(HTTPServerResponse::not_found) ;
        }
    }) ;

      app.addRoute("GET", "data/photos/{file}", [&app_context](HTTPServerRequest& req, HTTPServerResponse& resp) {  
        if ( !resp.serveStaticFile(app_context.web_root_ + "/public/data/photos/", req.getRouteAttribute("file")) ) {
             resp = HTTPServerResponse::stockReply(HTTPServerResponse::not_found) ;
        }
    }) ;

    app.registerBlueprint(admin) ;
    app.registerBlueprint(routes) ;
    app.registerBlueprint(api) ;

    
    server.run() ;
}
