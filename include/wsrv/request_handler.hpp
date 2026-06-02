#ifndef SERVER_REQUEST_HANDLER_HPP
#define SERVER_REQUEST_HANDLER_HPP

#include <string>

#include <wsrv/request.hpp>
#include <wsrv/response.hpp>

namespace ws {

/// Abstract handler for all incoming requests.
///

class SessionManager ;
class RequestHandler {
public:

    explicit RequestHandler() = default;
    RequestHandler(const RequestHandler &) = delete ;

    /// Override to handle a request and produce a response.

    virtual void handle(const HTTPServerRequest &req, HTTPServerResponse &resp) = 0;

    void setSessionManager(SessionManager *session_manager) {
        session_manager_.reset(session_manager) ;
    }

    SessionManager *sessionManager() { return session_manager_.get() ; }

    protected:
    std::unique_ptr<SessionManager> session_manager_ ;
};

} // namespace ws

#endif
