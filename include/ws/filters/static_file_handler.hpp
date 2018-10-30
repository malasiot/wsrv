#ifndef WS_STATIC_FILE_HANDLER_HPP
#define WS_STATIC_FILE_HANDLER_HPP

#include <ws/filter.hpp>

namespace ws {

class FilterChain ;
class Request ;
class Response ;

class StaticFileHandler: public Filter {
public:
    StaticFileHandler(const std::string &route_dir): root_(route_dir) {}

    void handle(Request &req, Response &resp, FilterChain &chain) override;

    std::string root_ ;
};


}

#endif
