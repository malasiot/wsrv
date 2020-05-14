#ifndef WS_SERVER_HPP
#define WS_SERVER_HPP

#include <string>
#include <vector>
#include <memory>

#include <ws/request_handler.hpp>
#include <ws/session_manager.hpp>

namespace ws {

class ServerImpl ;

class HttpServer {

public:
    /// Construct the server to listen on the specified TCP address and port, and
    /// serve up files from the given directory.
    explicit HttpServer(const std::string& address,
                    std::size_t io_service_pool_size = 4);

    ~HttpServer() ;

    // set request handler for this service

    void setHandler(RequestHandler *handler);

    /// Run the server's io_service loop.
    void run();

    /// Stop server loop
    void stop() ;

private:

    std::unique_ptr<ServerImpl> impl_ ;

};

} // namespace ws

#endif
