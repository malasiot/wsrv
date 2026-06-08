#pragma once

#include <wsrv/middleware.hpp>
#include <wsrv/util/logger.hpp>

namespace ws {

// Logs incoming request and response status + payload size
class RequestLogger: public IMiddleware {
public:
    RequestLogger(Logger &logger): logger_(logger) {}

    void handle(HTTPServerRequest& req, HTTPServerResponse& res, MiddlewareContext& ctx) ;

private:

    Logger &logger_ ;
};

}