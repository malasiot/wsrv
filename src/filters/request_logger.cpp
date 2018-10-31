#include <ws/filters/request_logger.hpp>

#include <ws/request.hpp>
#include <ws/response.hpp>
#include <ws/filter_chain.hpp>
#include <ws/exceptions.hpp>

#include <sstream>

using namespace std ;

namespace ws {

void RequestLoggerFilter::handle(Request &req, Response &resp, FilterChain &chain) {
    try {
        chain.next(req, resp) ;

        ostringstream strm ;
        strm << "Response to " <<
                 req.SERVER_.get("REMOTE_ADDR", "127.0.0.1")
                 << ": \"" << req.method_ << " " << req.path_
                 << ((req.query_.empty()) ? "" : "?" + req.query_) << " "
                 << req.protocol_ << "\" "
                 << resp.status_ << " " << resp.headers_.value<int>("Content-Length", 0)
                  ;
        if ( logger_ ) logger_->log(Logger::Info, strm.str()) ;
    }
    catch ( HttpResponseException &e ) {
        ostringstream strm ;
        strm << "Response to " <<
                 req.SERVER_.get("REMOTE_ADDR", "127.0.0.1")
                 << ": \"" << req.method_ << " " << req.path_
                 << ((req.query_.empty()) ? "" : "?" + req.query_) << " "
                 << req.protocol_ << "\" "
                 << resp.status_ ;
        if ( logger_ ) logger_->log(Logger::Info, strm.str()) ;

        throw e ;
    }
    catch ( std::exception &e ) {
        ostringstream strm ;
        strm << "Fatal error: " << e.what() ;
        if ( logger_ ) logger_->log(Logger::Error, strm.str()) ;
        throw e ;
    }
}


} // namespace ws
