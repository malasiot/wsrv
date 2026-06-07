#pragma once

#include <vector>
#include <wsrv/request_handler.hpp>
#include <wsrv/middleware.hpp>

namespace ws {

class Blueprint {
public:
    struct StagedRoute {
        StagedRoute(const std::string &method, const std::string &pat, const std::shared_ptr<RequestHandler> &handler, 
            const std::vector<IMiddlewarePtr> &mw, const std::string &name):
            method_(method), pattern_(pat), name_(name), handler_(handler), middlewares_(mw) {}

        std::string method_, pattern_, name_ ;
        std::shared_ptr<RequestHandler> handler_ ;
        std::vector<IMiddlewarePtr> middlewares_ ;
    };

private:
    std::string prefix_;
    std::vector<IMiddlewarePtr> group_middlewares_ ;
    std::vector<StagedRoute> routes_ ;
    
    std::vector<Blueprint> children_ ;

public:
    explicit Blueprint(std::string prefix) : prefix_(std::move(prefix)) {}

    Blueprint& use(const IMiddlewarePtr &mw) {
        group_middlewares_.push_back(mw);
        return *this;
    }

    // Stage a route inside the group
    Blueprint& addRoute(const std::string &method, const std::string &pattern, 
        const std::shared_ptr<RequestHandler> &handler, const std::vector<IMiddlewarePtr> &mws = {}, const std::string &name = {}) {
        routes_.emplace_back(method, pattern, handler, mws, name);
        return *this;
    }

    void addRoute(const std::string &method, const std::string& path, 
                   RequestHandlerCallable lambda_handler, 
                   const std::vector<std::shared_ptr<IMiddleware>> &route_mw = {},
                const std::string &name = {}) {
        
        // Wrap the incoming lambda inside our polymorphism bridge
        auto wrapped_handler = std::make_shared<RequestHandlerWrapper>(std::move(lambda_handler));
        
        // Save it using the exact same underlying logic
        routes_.emplace_back(method, path, wrapped_handler, route_mw, name);

        
    }

    // Nest a child blueprint inside this blueprint
    Blueprint& registerBlueprint(const Blueprint &child) {
        children_.emplace_back(child);
        return *this;
    }

    // Getters for recursive processing
    const std::vector<StagedRoute>& getRoutes() const { return routes_ ; }
    const std::vector<IMiddlewarePtr>& getGroupMiddleware() const { return group_middlewares_ ; }
    const std::string& getPrefix() const { return prefix_; }
    const std::vector<Blueprint>& getChildren() const { return children_; }
};


}