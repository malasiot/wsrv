
#include <wsrv/middleware.hpp>

namespace ws {
class CompressMiddleware: public IMiddleware {
public:
    void handle(HTTPServerRequest& req, HTTPServerResponse& res, MiddlewareContext& ctx) {
        ctx.next(req, res); // Move to next layer

        if ( req.supportsGzip() && res.contentBenefitsFromCompression() )
            res.compress();
    }
};
}