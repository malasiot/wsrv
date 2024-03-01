#ifndef SERVER_REQUEST_HANDLER_HPP
#define SERVER_REQUEST_HANDLER_HPP

#include <string>

#include <wsrv/request.hpp>
#include <wsrv/response.hpp>

namespace ws {

/// Abstract handler for all incoming requests.
///
class RequestHandler {
public:

    explicit RequestHandler() = default;
    RequestHandler(const RequestHandler &) = delete ;

    /// Override to handle a request and produce a response.

    virtual void handle(const HTTPServerRequest &req, HTTPServerResponse &resp) = 0;
};

} // namespace ws

#endif
