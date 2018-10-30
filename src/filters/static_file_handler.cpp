#include <ws/filters/static_file_handler.hpp>

#include <ws/request.hpp>
#include <ws/response.hpp>
#include <ws/filter_chain.hpp>

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem ;

namespace ws {

void StaticFileHandler::handle(Request &req, Response &resp, FilterChain &chain) {

    if ( resp.status_ != Response::ok && req.method_ == "GET" ) {
        fs::path p(root_ + req.path_)  ;
        if ( fs::exists(p) )
                resp.encodeFile(p.string());
    }

    chain.next(req, resp) ;
}


} // namespace wspp
