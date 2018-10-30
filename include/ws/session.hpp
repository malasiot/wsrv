#ifndef __SERVER_SESSION_HPP__
#define __SERVER_SESSION_HPP__

#include <string>
#include <ws/dictionary.hpp>
#include <ws/request.hpp>
#include <ws/response.hpp>

namespace ws {

class SessionHandler ;

class Session {
public:
    // start a new session
    Session(SessionHandler &handler, const Request &req, Response &resp, const std::string &suffix = std::string()) ;

    // closes the season
    ~Session() ;

    std::string id() const { return id_ ; }

    Dictionary &data() { return data_ ; }
    const Dictionary &data() const { return data_ ; }

private:

    std::string id_ ;
    Dictionary data_ ;
    uint64_t lifetime_ ;
    SessionHandler &handler_ ;
};

}



#endif
