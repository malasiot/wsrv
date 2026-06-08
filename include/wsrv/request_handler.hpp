#ifndef SERVER_REQUEST_HANDLER_HPP
#define SERVER_REQUEST_HANDLER_HPP

#include <string>
#include <functional>

#include <wsrv/request.hpp>
#include <wsrv/response.hpp>

namespace ws {

/// Abstract handler for all incoming requests.
///

class RequestHandler {
public:

    explicit RequestHandler() = default;
    RequestHandler(const RequestHandler &) = delete ;

    // Override to handle a request and produce a response.
    virtual void handle(HTTPServerRequest &req, HTTPServerResponse &resp) = 0;
};


using RequestHandlerCallable = std::function<void(HTTPServerRequest&, HTTPServerResponse&)>;

// Use to pass a functor as a handler
class RequestHandlerWrapper : public RequestHandler {
public:
    explicit RequestHandlerWrapper(RequestHandlerCallable callable) 
        : handler_func_(std::move(callable)) {}

    void handle(HTTPServerRequest& req, HTTPServerResponse& res) override {
        if (handler_func_) {
            handler_func_(req, res); // Forward execution to the lambda
        }
    }
private:
    RequestHandlerCallable handler_func_;
};


} // namespace ws

#endif
