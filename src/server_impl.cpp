#include "server_impl.hpp"
#include "detail/connection.hpp"
#include "detail/util.hpp"

namespace ws {

ServerImpl::ServerImpl
(const std::string& address,
               std::size_t io_service_pool_size)
    : io_service_pool_(io_service_pool_size),
      signals_(io_service_pool_.get_io_service()),
      acceptor_(io_service_pool_.get_io_service()),
      socket_(io_service_pool_.get_io_service())
{
    // Register to handle the signals that indicate when the server should exit.
    // It is safe to register for the same signal multiple times in a program,
    // provided all registration for the specified signal is made through Asio.
    signals_.add(SIGINT);
    signals_.add(SIGTERM);
#if defined(SIGQUIT)
    signals_.add(SIGQUIT);
#endif // defined(SIGQUIT)

    do_await_stop();

    auto atokens = split(address, ":") ;

    // Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
    asio::ip::tcp::resolver resolver(acceptor_.get_io_service());
    asio::ip::tcp::resolver::query query(atokens[0], atokens[1]);
    asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);

    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(asio::ip::tcp::acceptor::reuse_address(true));
    acceptor_.bind(endpoint);
    acceptor_.listen();

}


void ServerImpl::setHandler(RequestHandler *handler) {
    handler_.reset(handler) ;
}


void ServerImpl::run()
{
    start_accept();
    io_service_pool_.run();
}

void ServerImpl::start_accept()
{
    //new_connection_.reset(new connection(io_service_pool_.get_io_service(), handler_factory_));
    acceptor_.async_accept(socket_, [this] ( const std::error_code& e ){

        // Check whether the server was stopped by a signal before this
             // completion handler had a chance to run.
             if (!acceptor_.is_open())
             {
               return;
             }

             if (!e)
             {

               connection_manager_.start(std::make_shared<HttpConnection>(
                   std::move(socket_), connection_manager_, handler_.get()));
             }

        //if (!e) new_connection_->start();
        start_accept();
    }) ;
}

void ServerImpl::handle_stop()
{
    acceptor_.close();
    connection_manager_.stop_all();
    io_service_pool_.stop();
}


void ServerImpl::do_await_stop()
{
  signals_.async_wait(
      [this](std::error_code /*ec*/, int /*signo*/)
      {
        // The server is stopped by cancelling all outstanding asynchronous
        // operations. Once all operations have finished the io_service::run()
        // call will exit.
            handle_stop() ;
      });
}

void ServerImpl::stop()
{
    handle_stop() ;
}



} // namespace ws
