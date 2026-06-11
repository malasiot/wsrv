#pragma once

#include <variant/variant.hpp>
#include <wsrv/sqlite3_session_manager.hpp>
#include <wsrv/auth/authenticator.hpp>
#include <twig/loader.hpp>
#include <twig/renderer.hpp>
#include <twig/translator.hpp>

#include <memory>

class User: public ws::IAuthenticatedUser {
public:

    User(const std::string &id): id_(id) {} 
    virtual std::string getUniqueId() const override { return id_ ; };
private:
    std::string id_ ;
};

class SimpleAuthProvider: public ws::IAuthenticationProvider {
 virtual std::shared_ptr<ws::IAuthenticatedUser> authenticate(
        const std::string& username, 
        const std::string& password
    ) override {
        
        if ( username == "root" && password == "test" ) return make_shared<User>("1");
        else return nullptr ;
    }

    std::shared_ptr<ws::IAuthenticatedUser> loadUser(const std::string &id) override {
        return make_shared<User>(id) ;
    }
};

class AppContext {
public:

    AppContext(const Variant &cfg) ;

    string web_root_ ;
    std::shared_ptr<ws::SQLite3SessionManager> session_manager_ ;
    std::shared_ptr<twig::FileSystemTemplateLoader> loader_ ;
    std::shared_ptr<twig::TemplateRenderer> rdr_ ;
    std::shared_ptr<twig::TranslationManager> translator_ ;
    std::shared_ptr<SimpleAuthProvider> auth_ ;
    std::vector<std::string> locales_ ;
    std::string default_locale_, url_ ;
    size_t num_connections_ = 4 ;
};