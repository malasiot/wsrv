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

        if ( id_.empty() || !handler_.isValidId(id_)) { // new session ID needed
            id_ = handler_.uniqueSID() ;
            if ( id_.empty() ) { 
                session_started_ = false ;
                return ;
            }
            else
                set_cookie_ = true ; // we will need to set the cookie with the new session id
        }
        else // we have a valid id, we should read the data
            handler_.read(id_, data_) ;

        session_started_ = true ;
    }
}

Session::~Session() {
     if ( session_started_ ) {
        if ( data_.empty() ) { // we have an invalid session
            if ( !set_cookie_ ) { // this is not a new session id, we should delete it
                resp_.deleteCookie(key_name_, handler_.cookiePath(), handler_.cookieDomain(),
                                   handler_.cookieSecure(), handler_.cookieHttpOnly()) ;
                handler_.remove(id_) ;
            }
        } else {
            if ( set_cookie_ ) {
                int64_t expiration = std::chrono::duration_cast<std::chrono::seconds>(
                   std::chrono::system_clock::now().time_since_epoch()).count() + handler_.cookieExpiration() ;
                resp_.setCookie(key_name_, id_, expiration, handler_.cookiePath(), handler_.cookieDomain(),
                    handler_.cookieSecure(), handler_.cookieHttpOnly() ) ;
            }
            if ( modified_ )
                handler_.write(id_, data_) ;
        }

        handler_.close() ;
    }
}

} // namesace ws
