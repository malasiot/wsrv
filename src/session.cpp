#include <wsrv/session.hpp>
#include <wsrv/session_manager.hpp>

using namespace std ;

namespace ws {

Session::Session(SessionManager &handler, const HTTPServerRequest &req, HTTPServerResponse &resp, const std::string &suffix):
    handler_(handler), resp_(resp) {
    if ( handler_.open() ) {

        // check cookies and request args if session present

        key_name_ = "WSX_SESSION_ID" + suffix ;

        id_ = req.getCookie(key_name_) ;

        if ( id_.empty() || !handler_.isValidId(id_)) { // new session
            id_ = handler_.uniqueSID() ;
            if ( id_.empty() ) invalidate() ;
            set_cookie_ = true ;
        }
        else // we have a valid id
            handler_.read(*this) ;
    }
}

Session::~Session() {
    if ( status_ == STATUS_ACTIVE ) {
        if ( set_cookie_ )
            resp_.setCookie(key_name_, id_, handler_.cookieExpiration(), handler_.cookiePath(), handler_.cookieDomain(),
                handler_.cookieSecure(), handler_.cookieHttpOnly() ) ;
        handler_.write(*this) ;
        handler_.close() ;
    }
}

void Session::invalidate() {
    if ( status_ == STATUS_ACTIVE ) {
        data_.clear() ;
        status_ = STATUS_NONE ;
    }
}

} // namesace ws
