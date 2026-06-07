#include <wsrv/application.hpp>

using namespace std ;

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

std::string Application::url(const std::string &name, const Dictionary &params, bool relative) {
    auto it = named_routes_.find(name);
    if ( it == named_routes_.end() ) return "";
    else return it->second->url(params, relative) ; 
} 

void Application::registerBlueprint(const Blueprint &bp) {
    registerBlueprintRecursive(bp, "", {}) ;
}

std::string combine_paths(const std::string& base, const std::string& relative) {
    if (base.empty()) return relative;
    if (relative.empty()) return base;
        
    bool base_ends_slash = (base.back() == '/');
    bool rel_starts_slash = (relative.front() == '/');
        
    if (base_ends_slash && rel_starts_slash) {
        return base + relative.substr(1);
    } else if (!base_ends_slash && !rel_starts_slash) {
        return base + "/" + relative;
    } else  {
        return base + relative ;
    } 
}

  void Application::registerBlueprintRecursive(const Blueprint& bp, 
                              const std::string &current_prefix, 
                              std::vector<IMiddlewarePtr> accumulated_middleware) {
        
        // 1. Combine parent prefix with current blueprint prefix
        string prefix = combine_paths(current_prefix, bp.getPrefix());

        // 2. Append current blueprint middleware to the parent middleware chain
        accumulated_middleware.insert(
            accumulated_middleware.end(), 
            bp.getGroupMiddleware().begin(), 
            bp.getGroupMiddleware().end()
        );

        // Flatten and register all local routes at this level
        for (const auto& r : bp.getRoutes()) {
            std::string full_pattern = combine_paths(prefix, r.pattern_ );

            // Construct the final linear pipeline for ctx.next() vector
            std::vector<IMiddlewarePtr> final_pipeline = accumulated_middleware;
            final_pipeline.insert(final_pipeline.end(), r.middlewares_.begin(), r.middlewares_.end());

            addRoute(r.method_, full_pattern, r.handler_, final_pipeline);
        }

        //  Recursively traverse child blueprints
        for (const auto& child : bp.getChildren()) {
            registerBlueprintRecursive(child, current_prefix, accumulated_middleware);
        }
    }

   

}