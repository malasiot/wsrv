//
// HttpConnection.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef WS_CONNECTION_HPP
#define WS_CONNECTION_HPP


#include <ws/response.hpp>
#include <ws/request.hpp>
//#include <ws/util/logger.hpp>

#include <ws/request_handler.hpp>
#include <ws/session_manager.hpp>
#include <ws/exceptions.hpp>

#include "request_parser.hpp"
#include "connection_manager.hpp"

#include <asio/buffered_read_stream.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/write.hpp>

#include <iostream>

namespace ws {

class ConnectionManager ;
class ServerImpl ;


extern std::vector<asio::const_buffer> response_to_buffers(Response &rep, bool) ;

/// Represents a single HttpConnection from a client.

class HttpConnection:
        public std::enable_shared_from_this<HttpConnection>
{
public:
    explicit HttpConnection(asio::ip::tcp::socket socket,
                        ConnectionManager& manager,
                        RequestHandler *handler,
                            SessionManager *sm) : socket_(std::move(socket)),
        connection_manager_(manager), handler_(handler), request_(this), session_manager_(sm) {}

    Session &getSession() {
        if ( !session_ )
            session_.reset(new Session(*session_manager_, request_, response_)) ;

        return *session_ ;
    }
private:

    friend class ServerImpl ;
    friend class ConnectionManager ;



    void start() {
        read() ;
    }
    void stop() {
        socket_.close();
    }


    void read() {
        auto self(this->shared_from_this());
        socket_.async_read_some(asio::buffer(buffer_), [self, this] (std::error_code e, std::size_t bytes_transferred) {
            if (!e)
            {
                boost::tribool result;
                result = request_parser_.parse(buffer_.data(), bytes_transferred);

                if ( result )
                {
                    if ( !request_parser_.decode_message(request_) ) {
                        response_.stockReply(Response::bad_request);
                    }
                    else {
                        request_.SERVER_.add("REMOTE_ADDR", socket_.remote_endpoint().address().to_string() ) ;

                         try {
                             handler_->handle(request_, response_) ;

                         }

                        catch ( std::runtime_error &e ) {
                            response_.stockReply(Response::internal_server_error) ;
                        }
                    }

                    write(response_to_buffers(response_, request_.method_ == "HEAD")) ;

                }
                else if (!result)
                {
                    response_.stockReply(Response::bad_request);

                    write(response_to_buffers(response_, request_.method_ == "HEAD")) ;

                }
                else
                {
                    read() ;
                }
            }
            else if (e != asio::error::operation_aborted)
            {
                connection_manager_.stop(self) ;
            }

        });
    }

    void write(const std::vector<asio::const_buffer> &buffers)  {
        auto self(this->shared_from_this());
        asio::async_write(socket_, buffers, [this, self](asio::error_code e, std::size_t) {
            if (!e)
            {
                // Initiate graceful Connection closure.
                std::error_code ignored_ec;
                socket_.shutdown(asio::ip::tcp::socket::shutdown_both, ignored_ec);
            }

            if (e != asio::error::operation_aborted)
            {
                connection_manager_.stop(self) ;
            }
       });
    }



     asio::ip::tcp::socket socket_;

     /// The handler of incoming HttpRequest.
     RequestHandler *handler_;

     SessionManager *session_manager_ ;

     /// Buffer for incoming data.
     std::array<char, 8192> buffer_;

     ConnectionManager& connection_manager_ ;

      /// The parser for the incoming HttpRequest.
     detail::RequestParser request_parser_;

     Request request_ ;
     Response response_ ;

     std::unique_ptr<Session> session_ ;
};


} // namespace ws

#endif
