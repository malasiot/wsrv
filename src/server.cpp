#include <ws/server.hpp>

#include "server_impl.hpp"

namespace ws {

HttpServer::~HttpServer() = default ;

HttpServer::HttpServer(const std::string& address, const std::string& port,
               std::size_t io_service_pool_size): impl_(new ServerImpl(address, port, io_service_pool_size)) {
}


void HttpServer::setHandler(RequestHandler *handler) {
    impl_->setHandler(handler) ;

}

void HttpServer::run() {
    impl_->run() ;
}


void HttpServer::stop() {
    impl_->stop() ;
}

} // namespace ws
