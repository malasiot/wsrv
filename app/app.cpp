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
    auto auth = std::make_shared<SessionAuthController>(&session_manager, &provider);

    DebugLogger logger ;
  
    app.useGlobal(std::make_shared<RequestLogger>(logger)) ;

    Blueprint admin("admin/") ;

    admin.addRoute("GET|POST", "login/", [&](HTTPServerRequest& req, HTTPServerResponse& resp) {
        auto user_data = req.data().get("auth.user") ;
        auto locale = req.data().get("locale") ;
        rdr.setLocale(locale) ;

         if ( req.getMethod() == "GET" ) {
            resp.write(rdr.render("admin/login.html", 
                Variant::Object{{"app", Variant::Object{{
                    "locale", locale
                }}
            }}));
        } else {
            if ( !user_data.empty() )
                resp.json(R"({"success": true })");
            else
                resp.json(R"({"success": false, "message": "Invalid password or username credentials."})");
        }
        
    }, {locale, login} ) ;

     admin.addRoute("GET", "dashboard/", [&](HTTPServerRequest& req, HTTPServerResponse& resp) {
        auto user_data = req.data().get("auth.user") ;
        auto locale = req.data().get("locale") ;
        rdr.setLocale(locale) ;

       
            resp.write(rdr.render("admin/dashboard.html", 
                Variant::Object{
                    {"app", Variant::Object{{
                            "locale", locale
                             }}
                    }
                }));
        
        
    }, {locale} ) ;

     app.addRoute("GET", "/", [&](HTTPServerRequest& req, HTTPServerResponse& resp) {
        resp.redirect("/home/");
    } ) ;

    app.addRoute("GET", "home/", [&](HTTPServerRequest& req, HTTPServerResponse& resp) {
      
        auto locale = req.data().get("locale") ;
        rdr.setLocale(locale) ;


         resp.write(rdr.render("landing.html", 
            Variant::Object{{"app", Variant::Object{{
                "locale", locale
            }}
        }}));

  
    }, { locale } ) ;

      app.addRoute("GET", "routes/", [&](HTTPServerRequest& req, HTTPServerResponse& resp) {
        auto locale = req.data().get("locale") ;
        rdr.setLocale(locale) ;

         resp.write(rdr.render("routes.html", 
            Variant::Object{{"app", Variant::Object{{
                "locale", locale
            }}
        }}));

  
    }, { locale } ) ;
    app.addRoute("GET", "public/{file:.*}", [&session_manager, &rdr](HTTPServerRequest& req, HTTPServerResponse& resp) {
     //   resp.write(rdr.renderString("Requested file: {{file}}", { {"file", req.getRouteAttribute("file")} })) ;
      
        if ( !resp.serveStaticFile(string(web_root) + "public/", req.getRouteAttribute("file")) ) {
             resp = HTTPServerResponse::stockReply(HTTPServerResponse::not_found) ;
        }
    }) ;

    app.registerBlueprint(admin) ;

    
    server.run() ;
}
