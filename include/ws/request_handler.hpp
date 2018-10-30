#ifndef __SERVER_REQUEST_HANDLER_HPP__
#define __SERVER_REQUEST_HANDLER_HPP__

#include <string>

#include <ws/request.hpp>
#include <ws/response.hpp>

namespace ws {

/// Abstract handler for all incoming requests.
///
class RequestHandler {
public:

    explicit RequestHandler() = default;
    RequestHandler(const RequestHandler &) = delete ;

    /// Override to handle a request and produce a response.

    virtual void handle(const Request &req, Response &resp) = 0;
};


} // namespace ws

#endif
