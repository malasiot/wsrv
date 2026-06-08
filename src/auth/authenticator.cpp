#include <wsrv/auth/authenticator.hpp>
#include <wsrv/session.hpp>
#include <wsrv/auth/jwt_service.hpp>

namespace ws {

const char *USER_KEY = "user_id" ;
const char *USER_TOKEN = "user_token" ;

SessionAuthController::SessionAuthController(SessionManager *sm, IAuthenticationProvider *provider): 
    session_manager_(sm), provider_(provider) {
}

void SessionAuthController::handle(HTTPServerRequest &req, HTTPServerResponse &res, MiddlewareContext &ctx) {
    Session session(*session_manager_, req, res) ;

    std::string user_id = session.get(USER_KEY) ;
    if ( user_id.empty() ) {
        res = HTTPServerResponse::stockReply(HTTPServerResponse::forbidden) ;
        return ;
    }  else {
         std::shared_ptr<IAuthenticatedUser> user = provider_->loadUser(user_id);
        
        req.data().set("auth.user", user->getUniqueId());
    }
    ctx.next(req, res) ;
}

SessionLoginController::SessionLoginController(SessionManager *sm, IAuthenticationProvider *provider): 
    session_manager_(sm), provider_(provider) {

}

void SessionLoginController::handle(HTTPServerRequest &req, HTTPServerResponse &res, MiddlewareContext &ctx) {
    Session session(*session_manager_, req, res) ;

    if ( req.getMethod() == "POST" ) { // authentication form request
        std::string username = req.getPostAttribute("username") ;
        std::string password = req.getPostAttribute("password") ;
    
        auto user = provider_->authenticate(username, password) ;
        if ( user != nullptr ) { // user authenticated
            session.add(USER_KEY, user->getUniqueId()) ; // add session cookie
            req.data().set("auth.user", user->getUniqueId()) ;
        }

    } else { 
        std::string user_id = session.get(USER_KEY) ;
        if ( !user_id.empty() ) {
            auto user = provider_->loadUser(user_id) ;
            req.data().set("auth.user", user->getUniqueId()) ;
        }
    }
    ctx.next(req, res) ;
}

SessionLogoutController::SessionLogoutController(SessionManager *sm): 
    session_manager_(sm) {
}

void SessionLogoutController::handle(HTTPServerRequest &req, HTTPServerResponse &res, MiddlewareContext &ctx) {
    Session session(*session_manager_, req, res) ;

    std::string user_id = session.get(USER_KEY) ;

    if ( !user_id.empty() )
        session.remove(USER_KEY) ;
    
    req.data().remove("auth.user") ;
    

    ctx.next(req, res) ;
}

/////////////////////////


void JWTAuthController::handle(HTTPServerRequest &req, HTTPServerResponse &res, MiddlewareContext &ctx) {
    std::string auth_header = req.getServerAttribute("Authorization");
            
    if ( auth_header.rfind("Bearer ", 0) == 0 ) {
        std::string token = auth_header.substr(7); // Strip out "Bearer "
                
        std::string user_id = jwt_.verifyToken(token);
        if ( !user_id.empty() ) {
         //   req.getAppAttributes().emplace(USER_KEY, user_id) ;
        //    req.getAppAttributes().emplace(USER_TOKEN, token) ;
        }
    }
    ctx.next(req, res) ;
}

void JWTLoginController::handle(HTTPServerRequest &req, HTTPServerResponse &res, MiddlewareContext &ctx) {
    if ( req.getMethod() == "POST" ) { // authentication form request
        std::string username = req.getPostAttribute("username") ;
        std::string password = req.getPostAttribute("password") ;
    
        auto user = provider_->authenticate(username, password) ;
        if ( user != nullptr ) { // user authenticated
            std::string token = jwt_.generateToken(user);
             // 2. Package the token into a highly secure, engine-managed cookie configuration string
            res.setCookie("jwt_token", token, 900, "/", "", true, true) ;
       
          //  req.getAppAttributes().emplace(USER_KEY, user->getUniqueId()); // update app context
        }

    } else { 
         std::string auth_header = req.getServerAttribute("Authorization");
            
        if ( auth_header.rfind("Bearer ", 0) == 0 ) {
            std::string token = auth_header.substr(7); // Strip out "Bearer "
                
            std::string user_id = jwt_.verifyToken(token);
            if ( !user_id.empty() ) {
          //      req.getAppAttributes().emplace(USER_KEY, user_id) ;
            }
        }
    }

    ctx.next(req, res) ;
}

void JWTLogoutController::handle(HTTPServerRequest &req, HTTPServerResponse &res, MiddlewareContext &ctx) {
    res.addHeader("Set-Cookie", 
        "jwt_token=; "
        "Path=/; "
        "Expires=Thu, 01 Jan 1970 00:00:00 GMT; "
        "HttpOnly; Secure; SameSite=Strict");

    ctx.next(req, res) ;
}
}