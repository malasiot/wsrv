#include <wsrv/middleware/csrf.hpp>
#include <wsrv/crypto.hpp>
using namespace std ;
namespace ws {
// Prevents timing attacks targeting string comparisons
static bool constant_time_compare(const std::string& a, const std::string& b) {
    if (a.length() != b.length()) return false;
    unsigned char result = 0;
    for (size_t i = 0; i < a.length(); ++i) {
        result |= (a[i] ^ b[i]);
    }
    return result == 0;
}

static std::string generate_secure_token() {
    string code = encodeBase64(randomBytes(32)) ;
    std::replace_if(code.begin(), code.end(), [&](char c) {
        return !std::isalnum(c) ;
    }, '-') ;
    return code ;
}


void CSRFMiddleware::handle(HTTPServerRequest& req, HTTPServerResponse& res, MiddlewareContext &ctx) {
    // start new session
    Session session(*session_mgr_, req, res) ;

    // Skip validation for read-only  HTTP methods

    std::string method = req.getMethod() ;
    if ( method == "GET" || method == "HEAD" || method == "OPTIONS" ) {
         // Check if session needs a fresh token generated for the upcoming HTML view
        ensure_session_has_token(req, session);
            
        ctx.next(req, res); 
        return;
    }

    // Extract token from incoming request (check body form data first, then headers)
    std::string submitted_token = req.getPostAttribute("csrf_token");
    if ( submitted_token.empty() ) {
        submitted_token = req.getServerAttribute("X-CSRF-Token");
    }

    // Extract the legitimate token from the server-side session
    std::string session_token = session.get("csrf_token") ;

    // Securely validate the token presence and equality
    if ( submitted_token.empty() || session_token.empty() || 
            !constant_time_compare(submitted_token, session_token)) {
            
        // Short-circuit: Do NOT call next(). The request drops here.
        res = HTTPServerResponse::stockReply(HTTPServerResponse::forbidden) ;
        return;
    }

    req.data().set(make_shared<CSRFMiddlewareData>(session_token)) ;
    
    // Token is valid. Forward to next middleware or final route handler.
    ctx.next(req, res);
}

void CSRFMiddleware::ensure_session_has_token(HTTPServerRequest &req, Session &session) {
    string session_token = session.get("csrf_token") ;
    if ( session_token.empty() ) {
        session_token = generate_secure_token() ;
        session.add("csrf_token", session_token);
    }
    req.data().set(make_shared<CSRFMiddlewareData>(session_token)) ;
}

}