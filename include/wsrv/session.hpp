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
    // start a new session
    Session(SessionManager &handler, const Request &req, Response &resp, const std::string &suffix = std::string()) ;

    // closes the season
    ~Session() ;

    std::string id() const { return id_ ; }

    Dictionary &data() { return data_ ; }
    const Dictionary &data() const { return data_ ; }

    void add(const std::string &key, const std::string &val) {
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
        data_.erase(key) ;
    }

private:

    std::string id_ ;
    Dictionary data_ ;
    uint64_t lifetime_ ;
    SessionManager &handler_ ;
};

}



#endif
