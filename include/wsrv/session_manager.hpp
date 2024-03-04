#ifndef WS_SESSION_MANAGER_HPP
#define WS_SESSION_MANAGER_HPP

#include <string>
#include <chrono>

#include <wsrv/request.hpp>
#include <wsrv/response.hpp>
#include <wsrv/session.hpp>

namespace ws {

class Session ;

class SessionManager {

public:
    SessionManager(): session_cookie_path_("/") {}

    // initialize any resources
    virtual bool open() = 0 ;

    // close and release resources
    virtual bool close() = 0 ;

    // write session data (should invalidate session when session data is empty)
    virtual bool write(const Session &session) = 0 ;
    // read season data
    virtual bool read(Session &session) = 0 ;
    // generate a unique SID
    virtual std::string uniqueSID() { return generateSID() ; }

    virtual bool isValidId(const std::string &id) const = 0 ;

    virtual bool remove(const std::string &id) = 0 ;

    std::string cookiePath() const { return session_cookie_path_ ; }
    std::string cookieDomain() const { return session_cookie_domain_ ; }
    long int cookieExpiration() const { return session_cookie_expiration_ ; }
    long int sessionIdLifetime() const { return session_id_max_lifetime_ ; }
    bool cookieSecure() const { return session_cookie_secure_ ; }
    bool cookieHttpOnly() const { return session_cookie_http_only_ ; }

    void setCookiePath(const std::string &path) { session_cookie_path_ = path ; }
    void setCookieDomain(const std::string &domain) { session_cookie_domain_ = domain ; }
    void setCookieExpiration(long int expires) { session_cookie_expiration_ = expires ; }
    void setSessionLifetime(long int max_lifetime) { session_id_max_lifetime_ = max_lifetime ; }
    void setCookieSecure(bool secure) { session_cookie_secure_ = secure ; }
    void setCookieHttpOnly(bool http_only) { session_cookie_http_only_ = http_only ; }

protected:

     using Dictionary = std::map<std::string, std::string> ;

    static std::string generateSID() ;
    std::string session_cookie_path_, session_cookie_domain_ ;
    long int session_cookie_expiration_ = 0 ;
    long int session_id_max_lifetime_ = 60*24 ; // minutes
    bool session_cookie_http_only_ = false ;
    bool session_cookie_secure_ = false ;
};

} // namespace ws



#endif
