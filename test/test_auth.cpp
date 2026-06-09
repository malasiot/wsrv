#include <wsrv/server.hpp>
#include <wsrv/request_handler.hpp>
#include <wsrv/session.hpp>
#include <wsrv/exceptions.hpp>
#include <wsrv/sqlite3_session_manager.hpp>
#include <wsrv/middleware.hpp>

#include <wsrv/application.hpp>
#include <wsrv/auth/authenticator.hpp>

#include <mutex>
#include <iostream>
#include <regex>

#include <twig/renderer.hpp>

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


static const std::string LOGIN_FORM = R"(
<html><body>
<h1>Login</h1>
<h3>{{error}}</h3>
<form method='POST' >
<input type='text' name='username' />
<input type='text' name='password' />
<input type='submit' value='Upload' />
</form>
</body></html>
)";

int main(int argc, char *argv[]) {


    HttpServer server("127.0.0.1:5110") ;


    Application *app = new Application() ;
    
    server.setHandler(app) ;

    std::unique_ptr<SQLite3SessionManager> session_manager(new SQLite3SessionManager("/tmp/session.sqlite"));
  
    twig::TemplateRenderer rdr(nullptr) ;
  
    SimpleAuthProvider provider ;

    app->addRoute("GET|POST", "/login", [&rdr](HTTPServerRequest& req, HTTPServerResponse& resp) {
        auto user = req.data().get<IAuthenticatedUser>();
        
        if ( user ) {
            resp.write(rdr.renderString("<h1>Hello user {{id}}</h1><h3><a href=\"/logout/\">Logout</a> ", {{"id", user->getUniqueId()}})) ;
        }
        else {
            string error = "";
            if ( req.getMethod() == "POST" ) {
                error = "Authentication failed" ;
            }
            try {
             resp.write(rdr.renderString(LOGIN_FORM, {{"error", error}}));
            } catch ( std::exception &e ) {
                cout << e.what() << endl ;
            }
        }
  
    }, { std::make_shared<SessionLoginController>(session_manager.get(), &provider)}) ;

    app->addRoute("GET", "/logout", [](HTTPServerRequest& req, HTTPServerResponse& resp) {
        auto user = req.data().get<IAuthenticatedUser>();
        
        if ( user == nullptr ) 
            resp.write("<h1>Logout out succesfully</h1><h3><a href=\"/login/\">Login</a> ") ;
        else {
            resp.write("Logout failed");
        }
  
    }, { std::make_shared<SessionLogoutController>(session_manager.get())}) ;

     app->addRoute("GET", "/dashboard", [](HTTPServerRequest& req, HTTPServerResponse& resp) {
        auto user = req.data().get<IAuthenticatedUser>();
        
        if ( user ) 
            resp.write("<h1>User dashboard</h1><h3>" + user->getUniqueId() + "</h3>") ;

    }, { std::make_shared<SessionRequireAuth>(session_manager.get(), &provider)}) ;
  
  
    
    server.run() ;
}
