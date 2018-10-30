#ifndef WS_SERVER_HPP
#define WS_SERVER_HPP

#include <boost/asio.hpp>
#include <string>
#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

#include <ws/filter_chain.hpp>
#include <ws/detail/connection.hpp>
#include <ws/detail/io_service_pool.hpp>
#include <ws/detail/connection_manager.hpp>


namespace ws {

/// The top-level class of the HTTP server.
///
class Server {

public:
    /// Construct the server to listen on the specified TCP address and port, and
    /// serve up files from the given directory.
    explicit Server(const std::string& address, const std::string& port,
                    std::size_t io_service_pool_size = 4);

    // intercept filter/middleware to the service chain

    void addFilter(Filter *filter);

    // set request handler for this service

    void setHandler(RequestHandler *handler);

    /// Run the server's io_service loop.
    void run();

    /// Stop server loop
    void stop() ;

private:
    /// Initiate an asynchronous accept operation.
    void start_accept();

    /// Handle completion of an asynchronous accept operation.
    void handle_accept(const boost::system::error_code& e);

    /// Handle a request to stop the server.
    void handle_stop();

    void do_await_stop() ;

    /// The pool of io_service objects used to perform asynchronous operations.
    detail::io_service_pool io_service_pool_;

    /// The signal_set is used to register for process termination notifications.
    boost::asio::signal_set signals_;

    /// Acceptor used to listen for incoming connections.
    boost::asio::ip::tcp::acceptor acceptor_;

    ConnectionManager connection_manager_;

     /// The next socket to be accepted.
    boost::asio::ip::tcp::socket socket_;

    std::unique_ptr<RequestHandler> handler_ ;
    FilterChain filters_ ;
};

} // namespace ws

#endif
