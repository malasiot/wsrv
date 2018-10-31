#ifndef  WS_REQUEST_LOGGER_HPP
#define WS_REQUEST_LOGGER_HPP

#include <ws/filter.hpp>

namespace ws {

class FilterChain ;
class Request ;
class Response ;
class Logger ;

class RequestLoggerFilter: public Filter {
public:
    RequestLoggerFilter(Logger *logger): logger_(logger) {}

    void handle(Request &req, Response &resp, FilterChain &chain) override;

    Logger *logger_ ;
};


}

#endif
