#include <wsrv/application.hpp>

namespace ws {

void Application::handle(HTTPServerRequest& req, HTTPServerResponse& res) {
    std::vector<std::shared_ptr<IMiddleware>> global_pipeline;

    global_pipeline.reserve(global_middlewares_.size());
    for (const auto& mw : global_middlewares_) {
        global_pipeline.push_back(mw);
    }

    // Initialize the custom routing engine as the final destination
    RoutingEngineHandler router_destination(routes_);

    // Fire the pipeline! This ensures global middleware runs immediately.
    MiddlewareContext pipeline(global_pipeline, &router_destination);
    pipeline.next(req, res);
}

void Application::RoutingEngineHandler::handle(HTTPServerRequest &req, HTTPServerResponse &res) {
    // try to match the request with a route pattern. If no match is found, return 404
    const RouteEntry* matched = nullptr ;
    for ( auto &re: routes_ ) {
        Dictionary attrs ;
                
        if ( re.route_->matches(req.getPath(), attrs) && req.matchesMethod(re.method_) ) {
            req.getRouteAttributes() = std::move(attrs) ;
            matched = &re ;
            break ;
        }
    }

    if ( !matched ) {
        res = HTTPServerResponse::stockReply(HTTPServerResponse::not_found);
        return;
    }
            
    // If route-specific middleware exists, execute them dynamically
    if ( !matched->route_middlewares_.empty() ) {
        std::vector<std::shared_ptr<IMiddleware>> route_pipeline;
        for (const auto& mw : matched->route_middlewares_) {
            route_pipeline.push_back(mw);
        }
        // Inner context execution for localized layers
        MiddlewareContext inner_pipeline(route_pipeline, matched->handler_.get());
        inner_pipeline.next(req, res);
    } else {
        // Execute the route handler directly if no specific middleware
        matched->handler_->handle(req, res);
    }
}

}