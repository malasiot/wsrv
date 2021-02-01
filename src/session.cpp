#include <wsrv/session.hpp>
#include <wsrv/session_manager.hpp>

using namespace std ;

namespace ws {

Session::Session(SessionManager &handler, const Request &req, Response &resp, const std::string &suffix): handler_(handler) {
    if ( handler_.open() ) {

        // check cookies and request args if session present

        string key_name = "WSX_SESSION_ID" + suffix ;

        id_ = req.getCookie(key_name) ;
        if ( id_.empty() ) id_ = req.getQueryAttribute(key_name) ;
        if ( id_.empty() ) id_ = req.getPostAttribute(key_name) ;

        if ( id_.empty() ) { // new session
            id_ = handler_.uniqueSID() ;
            resp.setCookie(key_name, id_, 0, handler.cookiePath(), handler.cookieDomain() ) ;
        }
        else
            handler_.read(*this) ;
    }
}

Session::~Session() {
    handler_.write(*this) ;
    handler_.close() ;
}

} // namesace ws
