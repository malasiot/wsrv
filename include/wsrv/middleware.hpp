#pragma once

#include <wsrv/request.hpp>
#include <wsrv/response.hpp>
#include <wsrv/request_handler.hpp>
#include <typeindex>
#include <any>

namespace ws {

class MiddlewareContext ;

class IMiddleware {
public:
    virtual ~IMiddleware() = default;
    virtual void handle(HTTPServerRequest& req, HTTPServerResponse& res, MiddlewareContext& ctx) = 0;
};

using IMiddlewarePtr = std::shared_ptr<IMiddleware> ;

class MiddlewareContext {
public:
    MiddlewareContext(const std::vector<std::shared_ptr<IMiddleware>>& mw, RequestHandler *h)
        : middlewares_(mw), handler_(h) {}

   
    void next(HTTPServerRequest& req, HTTPServerResponse& res) {
        if (index_ < middlewares_.size()) {
            size_t current_index = index_++;
            middlewares_[current_index]->handle(req, res, *this);
        } else if (handler_) {
            handler_->handle(req, res);
        }
    }

private:
    const std::vector<std::shared_ptr<IMiddleware>>& middlewares_;
    RequestHandler *handler_;
    size_t index_ = 0;
};

class Pipeline {
public:
    void use(std::shared_ptr<IMiddleware> mw) {
        middlewares_.push_back(std::move(mw));
    }

    void setHandler(RequestHandler* h) {
        handler_ = h;
    }

    void run(HTTPServerRequest& req, HTTPServerResponse& res) {
        MiddlewareContext ctx(middlewares_, handler_);
        ctx.next(req, res); // Start the chain
    }

private:
    std::vector<std::shared_ptr<IMiddleware>> middlewares_;
    RequestHandler *handler_ = nullptr;
};
#if 0
 
#endif
}