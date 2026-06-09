#pragma once

#include <memory>
#include <wsrv/middleware.hpp>

namespace ws {
   
// model of authenticated user 
class IAuthenticatedUser {
public:
    virtual ~IAuthenticatedUser() = default;

    // return a unique id such as database row id
    virtual std::string getUniqueId() const = 0;
};

// should implement the authentication mechanism e.g. database authentication
class IAuthenticationProvider {
public:

    virtual ~IAuthenticationProvider() = default;
      
    // implement to authenticate the user and return a valid user pointer or null on failure
    virtual std::shared_ptr<IAuthenticatedUser> authenticate(
        const std::string& username, 
        const std::string& password
    ) = 0;

    virtual std::shared_ptr<IAuthenticatedUser> loadUser(const std::string &id) = 0 ;
};


class SessionManager ;

class SessionRequireAuth: public IMiddleware {
public:

    SessionRequireAuth(SessionManager *, IAuthenticationProvider *provider);

    void handle(HTTPServerRequest &req, HTTPServerResponse &res, MiddlewareContext &ctx) override ;

private:
    SessionManager *session_manager_ = nullptr ;
    IAuthenticationProvider *provider_ ;
};

class SessionCheckAuth: public IMiddleware {
public:

    SessionCheckAuth(SessionManager *, IAuthenticationProvider *provider);

    void handle(HTTPServerRequest &req, HTTPServerResponse &res, MiddlewareContext &ctx) override ;

private:
    SessionManager *session_manager_ = nullptr ;
    IAuthenticationProvider *provider_ ;
};

class SessionLoginController: public IMiddleware {
public:

    SessionLoginController(SessionManager *, IAuthenticationProvider *provider);

    void handle(HTTPServerRequest &req, HTTPServerResponse &res,  MiddlewareContext &ctx) override ;

private:
    SessionManager *session_manager_ = nullptr ;
    IAuthenticationProvider *provider_ ;

};

class SessionLogoutController: public IMiddleware {
public:

    SessionLogoutController(SessionManager *);

    void handle(HTTPServerRequest &req, HTTPServerResponse &res,  MiddlewareContext &ctx) override ;

private:
    SessionManager *session_manager_ = nullptr ;
};

class JWTService ;
class JWTAuthController: public IMiddleware {
public:

    JWTAuthController(JWTService &);

    void handle(HTTPServerRequest &req, HTTPServerResponse &res, MiddlewareContext &ctx) override ;

private:

    JWTService &jwt_ ;
};

class JWTLoginController: public IMiddleware {
public:

    JWTLoginController(JWTService &jwt, IAuthenticationProvider *provider): jwt_(jwt), provider_(provider) {}

    void handle(HTTPServerRequest &req, HTTPServerResponse &res,  MiddlewareContext &ctx) override ;

private:
    JWTService &jwt_ ;
    IAuthenticationProvider *provider_ ;
};

class JWTLogoutController: public IMiddleware {
public:

    JWTLogoutController(JWTService &jwt);

    void handle(HTTPServerRequest &req, HTTPServerResponse &res,  MiddlewareContext &ctx) override ;

private:
    JWTService &jwt_ ;
};


}