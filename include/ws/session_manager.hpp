#ifndef WS_SESSION_MANAGER_HPP
#define WS_SESSION_MANAGER_HPP

#include <string>
#include <ws/dictionary.hpp>
#include <ws/request.hpp>
#include <ws/response.hpp>
#include <ws/session.hpp>

namespace ws {

class Session ;

class SessionManager {
public:
    SessionManager(): session_cookie_path_("/") {}

    // initialize any resources
    virtual bool open() = 0 ;

    // close and release resources
    virtual bool close() = 0 ;

    // write session data
    virtual bool write(const Session &session) = 0 ;
    // read season data
    virtual bool read(Session &session) = 0 ;
    // generate a unique SID
    virtual std::string uniqueSID() { return generateSID() ; }

    std::string cookiePath() const { return session_cookie_path_ ; }
    std::string cookieDomain() const { return session_cookie_domain_ ; }

protected:

    static std::string generateSID() ;
    std::string session_cookie_path_, session_cookie_domain_ ;
};

} // namespace ws



#endif
