#include <wsrv/http_client.hpp>

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
    virtual void write(asio::streambuf &request) = 0;
    virtual size_t read(asio::streambuf &response, error_code &ec) = 0;


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


    void write(asio::streambuf &request) override {
        try {
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

    ConnectionSSL(asio::io_context &ios): ConnectionBase(ios), context_(asio::ssl::context::sslv23), ssl_socket_(socket_, context_) {}

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


    void write(asio::streambuf &request) override {
        try {
            asio::write(ssl_socket_, request);
        } catch ( exception &e ) {
            throw HTTPClientError(e.what()) ;
        }

    }

    size_t read(asio::streambuf &response, error_code &ec) override {
        return asio::read(ssl_socket_, response, ec) ;
    }

private:


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

    asio::ssl::context context_;
    asio::ssl::stream<asio::ip::tcp::socket&> ssl_socket_;
};

class HTTPClientImpl {
public:
    HTTPClientImpl() = default ;

    Response get(const URL &url) ;
    Response post(const URL &url, const std::map<std::string, std::string> &data);
    Response execute(const HTTPClientRequest &req) ;

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
    req.setBodyURLEncoded(data) ;
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
    if ( !req.body().empty() ) {
        request_stream << "Content-Length:" << req.body().size() << "\r\n" ;
        request_stream << "Content-Type: " << req.contentType() << "\r\n" ;
    }
    request_stream << "Accept: */*\r\n";
    request_stream << "Connection: close\r\n\r\n";

     if ( !req.body().empty() ) {
         request_stream << req.body() ;
     }
    con->write(request);

    Response res = readResponse(con.get()) ;

    return res ;
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
