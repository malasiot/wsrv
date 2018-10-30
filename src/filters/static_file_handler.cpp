#include <ws/filters/static_file_handler.hpp>

#include <ws/request.hpp>
#include <ws/response.hpp>
#include <ws/filter_chain.hpp>

#include "detail/filesystem.hpp"

using namespace std ;

namespace ws {

void StaticFileHandler::handle(Request &req, Response &resp, FilterChain &chain) {

    if ( resp.status_ != Response::ok && req.method_ == "GET" ) {
        string p(root_ + '/' + req.path_) ;
        if ( fileExists(p) )
                resp.encodeFile(p);
    }

    chain.next(req, resp) ;
}


} // namespace wspp
