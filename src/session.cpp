#include <wsrv/session.hpp>
#include <wsrv/session_manager.hpp>

using namespace std ;

namespace ws {

Session::Session(SessionManager &handler, const HTTPServerRequest &req, const std::string &suffix):
    handler_(handler) {
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
            handler_.read(id_, data_) ;

        status_ = STATUS_ACTIVE;
    }
}

void Session::writeCookie(HTTPServerResponse &resp) const{
     if ( status_ == STATUS_ACTIVE ) {
        if ( data_.empty() ) { // we have an invalid session
            if ( !set_cookie_ ) { // this is not a new session id, we should delete it
                resp.deleteCookie(key_name_, handler_.cookiePath(), handler_.cookieDomain(),
                                   handler_.cookieSecure(), handler_.cookieHttpOnly()) ;
                handler_.remove(id_) ;
            }
        } else {
            if ( set_cookie_ )
                resp.setCookie(key_name_, id_, handler_.cookieExpiration(), handler_.cookiePath(), handler_.cookieDomain(),
                    handler_.cookieSecure(), handler_.cookieHttpOnly() ) ;
            if ( modified_ )
                handler_.write(id_, data_) ;
        }

        handler_.close() ;
    }
}

Session::~Session() {
 
}

void Session::invalidate() {
    if ( status_ == STATUS_ACTIVE ) {
        data_.clear() ;
        status_ = STATUS_NONE ;
    }
}

} // namesace ws
