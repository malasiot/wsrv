#include <ws/server.hpp>

#include "server_impl.hpp"

namespace ws {

Server::~Server() = default ;

Server::Server(const std::string& address, const std::string& port,
               std::size_t io_service_pool_size): impl_(new ServerImpl(address, port, io_service_pool_size)) {
}

void Server::addFilter(Filter *filter) {
    impl_->addFilter(filter) ;
}

void Server::setHandler(RequestHandler *handler) {
    impl_->setHandler(handler) ;

}

void Server::run() {
    impl_->run() ;
}


void Server::stop() {
    impl_->stop() ;
}



} // namespace ws
