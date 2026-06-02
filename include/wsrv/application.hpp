#pragma once

#include <functional>

#include <wsrv/request.hpp>
#include <wsrv/response.hpp>
#include <wsrv/request_handler.hpp>
#include <wsrv/middleware.hpp>
#include <wsrv/route.hpp>

namespace ws {

using RequestHandlerCallable = std::function<void(HTTPServerRequest&, HTTPServerResponse&)>;

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

// implementation of the main application handler that supports routing and middleware

class Application: public RequestHandler {
public:
    using Dictionary = std::map<std::string, std::string>;

    Application() = default ;

    // main entry point for the server
    void handle(HTTPServerRequest& req, HTTPServerResponse& res) override ;

    // add a global middleware that will be executed for all routes. This is useful for things like logging, authentication etc.
    void useGlobal(const std::shared_ptr<IMiddleware> &mw) {
        global_middlewares_.push_back(mw);
    }

    // add a route with a handler and optional route-specific middleware. The handler can be either a lambda or a class that inherits from RequestHandler.
    void addRoute(const std::string &method, const std::string& path, const std::shared_ptr<RequestHandler> &handler, const std::vector<std::shared_ptr<IMiddleware>>& middlewares = {}) {
        routes_.emplace_back(method, path, std::move(handler), std::move(middlewares)) ;
    }

    void addRoute(const std::string &method, const std::string& path, 
                   RequestHandlerCallable lambda_handler, 
                   const std::vector<std::shared_ptr<IMiddleware>> &route_mw = {}) {
        
        // Wrap the incoming lambda inside our polymorphism bridge
        auto wrapped_handler = std::make_shared<RequestHandlerWrapper>(std::move(lambda_handler));
        
        // Save it using the exact same underlying logic
        routes_.emplace_back(method, path, std::move(wrapped_handler), std::move(route_mw));
    }

private:

    struct RouteEntry {
        RouteEntry(const std::string &m, const std::string &p, const std::shared_ptr<RequestHandler> &h, const std::vector<std::shared_ptr<IMiddleware>> &mw = {})
            : method_(m), pattern_(p), handler_(h), route_middlewares_(mw) {
                route_ = std::make_unique<Route>(pattern_) ;
            }

        std::unique_ptr<Route> route_ ; // compiled route pattern for efficient matching
        std::string method_, pattern_ ;
        std::shared_ptr<RequestHandler> handler_;
        std::vector<std::shared_ptr<IMiddleware>> route_middlewares_;
    };

    // This is the final handler in the pipeline that performs route matching and executes the appropriate handler for each route
    class RoutingEngineHandler: public RequestHandler {
    private:
        const std::vector<RouteEntry>& routes_;
    public:
        explicit RoutingEngineHandler(const std::vector<RouteEntry>& r) : routes_(r) {}

        void handle(HTTPServerRequest &req, HTTPServerResponse &res) override ;
    };

    std::vector<std::shared_ptr<IMiddleware>> global_middlewares_;
    std::vector<RouteEntry> routes_;
};

}