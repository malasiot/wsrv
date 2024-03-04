#ifndef WS_SERVER_SESSION_HPP
#define WS_SERVER_SESSION_HPP

#include <string>

#include <wsrv/request.hpp>
#include <wsrv/response.hpp>

namespace ws {

class SessionManager ;

class Session {
    using Dictionary = std::map<std::string, std::string>;
public:

    enum Status { STATUS_NONE, STATUS_ACTIVE } ;

    // start a new session
    Session(SessionManager &handler, const HTTPServerRequest &req, HTTPServerResponse &resp, const std::string &suffix = std::string()) ;

    // closes the season
    ~Session() ;

    std::string id() const { return id_ ; }

    // if modifying the data should call setModified(true) ;
    Dictionary &data() { return data_ ; }
    const Dictionary &data() const { return data_ ; }

    void add(const std::string &key, const std::string &val) {
        modified_ = true ;
        data_.emplace(key, val) ;
    }

    std::string get(const std::string &key, const std::string &def = std::string()) {
        auto it = data_.find(key) ;
        return it == data_.end() ? def : it->second ;
    }

    bool contains(const std::string &key) {
        return data_.find(key) != data_.end() ;
    }

    void remove(const std::string &key) {
        modified_ = true;
        data_.erase(key) ;
    }

    void invalidate() ;

    // clearing the data will delete them from the server
    void clear() {
        modified_ = true ;
        data_.clear() ;
    }

    void setModified(bool m) {
        modified_ = m ;
    }

private:

    std::string id_ ;
    Dictionary data_ ;
    uint64_t lifetime_ ;
    SessionManager &handler_ ;
    HTTPServerResponse &resp_ ;
    Status status_ = STATUS_NONE ;
    bool set_cookie_ = false ;
    std::string key_name_ ;
    bool modified_ = false ;
};

}



#endif
