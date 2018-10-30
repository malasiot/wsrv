#ifndef WS_SERVER_HPP
#define WS_SERVER_HPP

#include <string>
#include <vector>
#include <memory>

#include <ws/filter_chain.hpp>


namespace ws {

class ServerImpl ;

class Server {

public:
    /// Construct the server to listen on the specified TCP address and port, and
    /// serve up files from the given directory.
    explicit Server(const std::string& address, const std::string& port,
                    std::size_t io_service_pool_size = 4);

    ~Server() ;
    // intercept filter/middleware to the service chain

    void addFilter(Filter *filter);

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
