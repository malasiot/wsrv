#ifndef WS_SERVER_IMPL_HPP
#define WS_SERVER_IMPL_HPP

#include <asio/signal_set.hpp>

#include <string>
#include <vector>

#include <ws/request_handler.hpp>
#include <ws/session_manager.hpp>

#include "detail/connection.hpp"
#include "detail/io_service_pool.hpp"
#include "detail/connection_manager.hpp"


namespace ws {

class ServerImpl {

public:
    /// Construct the server to listen on the specified TCP address and port, and
    /// serve up files from the given directory.
    explicit ServerImpl(const std::string& address, const std::string& port,
                    std::size_t io_service_pool_size = 4);


    // set request handler for this service

    void setHandler(RequestHandler *handler);

    void setSessionManager(SessionManager *mgr) ;


    /// Run the server's io_service loop.
    void run();

    /// Stop server loop
    void stop() ;

private:
    /// Initiate an asynchronous accept operation.
    void start_accept();

    /// Handle completion of an asynchronous accept operation.
    void handle_accept(const std::error_code& e);

    /// Handle a request to stop the server.
    void handle_stop();

    void do_await_stop() ;

    /// The pool of io_service objects used to perform asynchronous operations.
    detail::io_service_pool io_service_pool_;

    /// The signal_set is used to register for process termination notifications.
    asio::signal_set signals_;

    /// Acceptor used to listen for incoming connections.
    asio::ip::tcp::acceptor acceptor_;

    ConnectionManager connection_manager_;

     /// The next socket to be accepted.
    asio::ip::tcp::socket socket_;

    std::unique_ptr<RequestHandler> handler_ ;
    std::unique_ptr<SessionManager> session_manager_ ;

};

} // namespace ws

#endif
