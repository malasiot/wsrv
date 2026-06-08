#pragma once
#include <wsrv/middleware.hpp>
#include <wsrv/session.hpp>

namespace ws {

class CSRFMiddleware : public IMiddleware {
    SessionManager *session_mgr_ ;
public:
    CSRFMiddleware(SessionManager *session_mgr): session_mgr_(session_mgr) {}
    void handle(HTTPServerRequest& req, HTTPServerResponse& res, MiddlewareContext &ctx) override ;

private:
    void ensure_session_has_token(HTTPServerRequest& req, Session &session) ;  
};

// data stored in the request to be used for forms 
class CSRFMiddlewareData {
public:
    CSRFMiddlewareData(const std::string &token): token_(token) {}

    const std::string &token() const { return token_ ; }
private:
    std::string token_ ;
};

}