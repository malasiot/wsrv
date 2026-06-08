#include <wsrv/middleware/request_logger.hpp>

using namespace std ;

namespace ws {

void RequestLogger::handle(HTTPServerRequest& req, HTTPServerResponse& res, MiddlewareContext& ctx) {
    ctx.next(req, res); 
    
    std::stringstream strm ;
       
    strm << req.toString() << " -> " << res.toString() ;

    logger_.log(LogLevel::Info, strm.str()) ;
}

}