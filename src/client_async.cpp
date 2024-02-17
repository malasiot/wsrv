#include <wsrv/client.hpp>

#include <asio/io_service.hpp>
#include <asio/connect.hpp>
#include <asio/write.hpp>
#include <asio/read_until.hpp>
#include <asio/streambuf.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/read.hpp>
#include <asio/ssl.hpp>
#include <asio/placeholders.hpp>

#include <iostream>
#include <wsrv/response.hpp>

#include "detail/response_parser.hpp"

using namespace std ;
using namespace asio ;
using namespace asio::ip ;

namespace ws {

class ConnectionBase {

public:
    ConnectionBase(asio::io_context &ios): socket_(ios), resolver_(ios) {}

    virtual void connect(const string &host) = 0 ;
    virtual void connectAsync(const HTTPClientRequest &req) = 0 ;
    virtual void write(asio::streambuf &request) = 0;
    virtual size_t read(asio::streambuf &response, error_code &ec) = 0;

    asio::streambuf &buffer() { return strmbuf_ ; }

    virtual ~ConnectionBase()
    {
        try
        {
            socket_.close();
        }
        catch (...)
        {
        }
    }

protected:

    tcp::resolver resolver_ ;
    asio::ip::tcp::socket socket_;
    asio::streambuf strmbuf_, response_;
};

class Connection: public ConnectionBase {
public:

    Connection(asio::io_context &ios): ConnectionBase(ios) {}

    void connect(const std::string &host) override {
        try {
            tcp::resolver::query query(host, "http");
            tcp::resolver::iterator endpoint_iterator = resolver_.resolve(query);

            asio::connect(socket_, endpoint_iterator);
        } catch ( exception &e ) {
            throw HTTPClientError(e.what()) ;
        }
    }

    void connectAsync(const HTTPClientRequest &req) override {
    }


    void write(asio::streambuf &request) override {
        try {
            std::ostream request_stream(&request);

            asio::write(socket_, request);
        } catch ( exception &e ) {
            throw HTTPClientError(e.what()) ;
        }
    }
    size_t read(asio::streambuf &response, error_code &ec) override  {
        return asio::read(socket_, response, ec) ;
    }


};

class ConnectionSSL: public ConnectionBase {
public:

    ConnectionSSL(asio::io_context &ios): ConnectionBase(ios), context_(asio::ssl::context::sslv23), ssl_socket_(socket_, context_), read_buf_(new char[buf_size_]) {}

    void connect(const string &host) override {
        try {

            tcp::resolver::query query(host, "https");
            tcp::resolver::iterator endpoint_iterator = resolver_.resolve(query);

            ssl_socket_.set_verify_mode(ssl::verify_none);

            asio::connect(ssl_socket_.lowest_layer(), endpoint_iterator);
            ssl_socket_.handshake(ssl::stream_base::client) ;
        } catch ( exception &e ) {
            throw HTTPClientError(e.what()) ;
        }
    }


    void connectAsync(const HTTPClientRequest &req) override {
        try {

        tcp::resolver::query query(req.url().host(), "https");
        resolver_.async_resolve(query, std::bind(&ConnectionSSL::handle_resolve, this,
                                                                               std::placeholders::_1,
                                                                               std::placeholders::_2));
        }
        catch ( exception &e ) {
                    throw HTTPClientError(e.what()) ;
                }

    }

    void write(asio::streambuf &request) override {
        try {
            std::ostream request_stream(&request);
            asio::write(ssl_socket_, request);
        } catch ( exception &e ) {
            throw HTTPClientError(e.what()) ;
        }

    }

    size_t read(asio::streambuf &response, error_code &ec) override {
        return asio::read(ssl_socket_, response, ec) ;
    }

private:

    void handle_read_content(const std::error_code& err) {
        std::cout << "handle_write_request start \n";
        std::cout << "handle_read_status_line start \n";
              if (!err)
              {
                  int parser_result = parser_.parse(asio::buffer_cast<const char *>(response_.data()), response_.size()) ;

                  if ( parser_result == detail::HTTP_PARSER_ERROR )
                      throw HTTPClientError("Invalid response received") ;
                   else if ( parser_result == detail::HTTP_PARSER_INDETERMINATE ) {
                      asio::async_read(ssl_socket_, response_,
                              asio::transfer_at_least(1),
                              std::bind(&ConnectionSSL::handle_read_content, this,
                                std::placeholders::_1));
                  } else {
                      cout << "response OK" << endl;
                      Response res ;
                      parser_.decode_message(res) ;
                      cout << res.getStatus() << endl ;
                  }

              }
              else
              {
                  std::cout << "Error: " << err.message() << "\n";
              }
    }

    void handle_read_status_line(const std::error_code& err) {
         if (!err) {
             // Check that response is OK.

              int parser_result = parser_.parse(asio::buffer_cast<const char *>(response_.data()), response_.size()) ;

              if ( parser_result == detail::HTTP_PARSER_ERROR )
                  throw HTTPClientError("Invalid response received") ;
               else if ( parser_result == detail::HTTP_PARSER_INDETERMINATE ) {
                  asio::async_read(ssl_socket_, response_,
                          asio::transfer_at_least(1),
                          std::bind(&ConnectionSSL::handle_read_content, this,
                            std::placeholders::_1));
              } else {
                  cout << "response OK" << endl;
                  Response res ;
                  parser_.decode_message(res) ;
                  cout << res.getStatus() << endl ;
              }

         }
    }

    void handle_write_request(const std::error_code& err) {
        std::cout << "handle_write_request start \n";
              if (!err)
              {
                  // Read the response status line. The response_ streambuf will
                  // automatically grow to accommodate the entire line. The growth may be
                  // limited by passing a maximum size to the streambuf constructor.
                  asio::async_read_until(ssl_socket_, response_, "\r\n\r\n",
                                                std::bind(&ConnectionSSL::handle_read_status_line, this,
                                                            std::placeholders::_1));

          /*        ssl_socket_.async_read_some(
                             asio::buffer(read_buf_.get(), buf_size_),
                             std::bind(
                                 &ConnectionSSL::handle_read, this,
                                 std::placeholders::_1,
                                 std::placeholders::_2
                             )
                         );
                         */
              }
              else
              {
                  std::cout << "Error write req: " << err.message() << "\n";
              }
    }

    void handle_handshake(const std::error_code& error)
       {
           std::cout << "handle_handshake start \n";
           if (!error)
           {
               std::cout << "Handshake OK " << "\n";
               std::cout << "Request: " << "\n";


               // The handshake was successful. Send the request.
               asio::async_write(ssl_socket_, strmbuf_,
                                        std::bind(&ConnectionSSL::handle_write_request, this,
                                                    std::placeholders::_1));
           }
           else
           {
               std::cout << "Handshake failed: " << error.message() << "\n";
           }
       }

    void handle_connect(const std::error_code& err) {
        std::cout << "handle_connect\n";
               if (!err)
               {
                   std::cout << "Connect OK " << "\n";
                   ssl_socket_.async_handshake(asio::ssl::stream_base::client,
                                           std::bind(&ConnectionSSL::handle_handshake, this,
                                                       std::placeholders::_1));
               }
               else
               {
                   std::cout << "Connect failed: " << err.message() << "\n";
               }
    }


  void handle_resolve(const std::error_code& err,
      tcp::resolver::iterator endpoint_iterator)
  {
    if (!err)
    {
         ssl_socket_.set_verify_mode(ssl::verify_none);
         // Set SNI Hostname (many hosts need this to handshake successfully)
         /*     if(! SSL_set_tlsext_host_name(ssl_socket_.native_handle(), "stackoverflow.com"))
              {

              }
*/
         asio::async_connect(ssl_socket_.lowest_layer(), endpoint_iterator,
                                            std::bind(&ConnectionSSL::handle_connect, this,
                                                        std::placeholders::_1));
    }
    else
    {
      std::cout << "Error: " << err.message() << "\n";
    }
  }
    virtual ~ConnectionSSL()
    {
        try
        {
            socket_.close();
        }
        catch ( exception &e )
        {
            throw HTTPClientError(e.what()) ;
        }
    }

private:

    detail::ResponseParser parser_ ;
    asio::ssl::context context_;
    asio::ssl::stream<asio::ip::tcp::socket&> ssl_socket_;
    static const std::size_t buf_size_ = 512u;
       std::unique_ptr<char[]> read_buf_;
};

class HTTPClientImpl {
public:
    HTTPClientImpl() = default ;

    Response get(const URL &url) ;
    Response post(const URL &url, const std::map<std::string, std::string> &data);
    Response execute(const HTTPClientRequest &req) ;
     void executeAsync(const HTTPClientRequest &req) ;


private:

    friend class HTTPClient ;

    void writeHeaders(std::ostream &strm, const HTTPClientRequest &req);
    ConnectionBase *connect(const URL &url);
    Response readResponse(ConnectionBase *con);

    asio::io_context ios_;
};



ConnectionBase *HTTPClientImpl::connect(const URL &url) {
    ConnectionBase *connection = nullptr;
    if ( url.protocol() == "http" )
        connection = new Connection(ios_) ;
    else if ( url.protocol() == "https" )
        connection = new ConnectionSSL(ios_) ;

    return connection ;
}


Response HTTPClientImpl::readResponse(ConnectionBase *con) {

    detail::ResponseParser parser ;

    size_t sz ;
    int parser_result ;

    error_code ec;

    static const size_t buf_sz = 1024 * 16 ;

    do {
        asio::streambuf response(buf_sz);

        sz = con->read(response, ec) ;

        parser_result = parser.parse(asio::buffer_cast<const char*>( response.data() ), response.size()) ;

        if ( parser_result == detail::HTTP_PARSER_ERROR )
            throw HTTPClientError("Invalid response received") ;

    } while ( !ec && ec != asio::error::eof && parser_result != detail::HTTP_PARSER_OK ) ;

    Response resp ;
    parser.decode_message(resp) ;

    return resp ;

}

Response HTTPClientImpl::get(const URL &url) {
    HTTPClientRequest req(url) ;
    req.setMethod(HTTPClientRequest::GET) ;
    return execute(req) ;
}

Response HTTPClientImpl::post(const URL &url, const std::map<std::string, std::string> &data) {

    HTTPClientRequest req(url) ;
    req.setMethod(HTTPClientRequest::POST) ;
    req.setBody(new URLEncodedRequestBody(data)) ;
    return execute(req) ;
}

void HTTPClientImpl::writeHeaders(std::ostream &strm, const HTTPClientRequest &req) {
    for( const auto &kv: req.headers() ) {
        strm << kv.first << ':' << kv.second << "\r\n" ;
    }
}

Response HTTPClientImpl::execute(const HTTPClientRequest &req)
{
    const auto &url = req.url() ;
    std::unique_ptr<ConnectionBase> con(connect(url)) ;

    if ( con ) con->connect(url.host()) ;

    asio::streambuf request;
    std::ostream request_stream(&request);

    request_stream << req.methodString() << " /" << url.file() << " HTTP/1.1\r\n";
    request_stream << "Host: " << url.host() << "\r\n";

    writeHeaders(request_stream, req) ;
    if ( req.body() ) {
        request_stream << "Content-Length:" << req.body()->contentLength() << "\r\n" ;
        request_stream << "Content-Type: " << req.body()->contentType() << "\r\n" ;
    }
    request_stream << "Accept: */*\r\n";
    request_stream << "Connection: close\r\n\r\n";

     if ( req.body() ) {
         req.body()->write(request_stream) ;
     }
    con->write(request);

    Response res = readResponse(con.get()) ;

    return res ;
}

void HTTPClientImpl::executeAsync(const HTTPClientRequest &req)
{
    const auto &url = req.url() ;
    ConnectionBase *con = connect(url) ;

    asio::streambuf &request = con->buffer() ;
    std::ostream request_stream(&request);

    request_stream << req.methodString() << " /" << url.file() << " HTTP/1.1\r\n";
    request_stream << "Host: " << url.host() << "\r\n";

    writeHeaders(request_stream, req) ;
    if ( req.body() ) {
        request_stream << "Content-Length:" << req.body()->contentLength() << "\r\n" ;
        request_stream << "Content-Type: " << req.body()->contentType() << "\r\n" ;
    }
    request_stream << "Accept: */*\r\n";
    request_stream << "Connection: close\r\n\r\n";

     if ( req.body() ) {
         req.body()->write(request_stream) ;
     }

    con->connectAsync(req);

    ios_.run() ;


}


HTTPClient::HTTPClient(): impl_(new HTTPClientImpl()) {
}

HTTPClient::~HTTPClient()
{

}

Response HTTPClient::execute(HTTPClientRequest &req)
{
    assert(impl_) ;
    return impl_->execute(req) ;
}

void HTTPClient::executeAsync(HTTPClientRequest &req)
{
    assert(impl_) ;
    impl_->executeAsync(req) ;
}

Response HTTPClient::get(const string &url)
{
    assert(impl_) ;
    return impl_->get(url) ;
}

Response HTTPClient::post(const string &url, const std::map<string, string> &data)
{
    assert(impl_) ;
    return impl_->post(url, data) ;
}




}
