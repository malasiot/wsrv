#include <wsrv/client.hpp>

#include <asio/io_service.hpp>
#include <asio/connect.hpp>
#include <asio/write.hpp>
#include <asio/read_until.hpp>
#include <asio/streambuf.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/read.hpp>
#include <asio/ssl.hpp>

#include <iostream>
#include <wsrv/response.hpp>

#include "detail/response_parser.hpp"

using namespace std ;
using namespace asio ;
using namespace asio::ip ;

namespace ws {

class ConnectionBase {

public:
    ConnectionBase(): socket_(ios_) {}

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

    asio::io_context ios_;
    asio::ip::tcp::socket socket_;
    asio::streambuf strmbuf_;
};

class Connection: public ConnectionBase {
public:

    Connection(): ConnectionBase() {}

    void connect(const string &host) override {
        try {
            tcp::resolver resolver(ios_);
            tcp::resolver::query query(host, "http");
            tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

            asio::connect(socket_, endpoint_iterator);
        } catch ( exception &e ) {
            throw HTTPClientError(e.what()) ;
        }
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

    ConnectionSSL(): context_(asio::ssl::context::sslv23), ssl_socket_(socket_, context_) {}

    void connect(const string &host) override {
        try {
            tcp::resolver resolver(ios_);
            tcp::resolver::query query(host, "https");
            tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

            ssl_socket_.set_verify_mode(ssl::verify_none);

            asio::connect(ssl_socket_.lowest_layer(), endpoint_iterator);
            ssl_socket_.handshake(ssl::stream_base::client) ;
        } catch ( exception &e ) {
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

    HTTPServerResponse get(const URL &url) ;
    HTTPServerResponse post(const URL &url, const std::map<std::string, std::string> &data);

private:

    friend class HTTPClient ;

    ConnectionBase *connect(const URL &url);
    HTTPServerResponse readResponse(ConnectionBase *con);

    string host_name_ ;
};



ConnectionBase *HTTPClientImpl::connect(const URL &url) {
    ConnectionBase *connection = nullptr;
    if ( url.protocol() == "http" )
        connection = new Connection() ;
    else if ( url.protocol() == "https" )
        connection = new ConnectionSSL() ;

    if ( connection ) connection->connect(url.host()) ;
    return connection ;
}


HTTPServerResponse HTTPClientImpl::readResponse(ConnectionBase *con) {

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

    HTTPServerResponse resp ;
    parser.decode_message(resp) ;

    return resp ;

}

HTTPServerResponse HTTPClientImpl::get(const URL &url) {

    std::unique_ptr<ConnectionBase> con(connect(url)) ;

    // Form the request. We specify the "Connection: close" header so that the
    // server will close the socket after transmitting the response. This will
    // allow us to treat all data up until the EOF as the content.

    asio::streambuf request;
    std::ostream request_stream(&request);

    request_stream << "GET " << url.file() << " HTTP/1.1\r\n";
    request_stream << "Host: " << url.host() << "\r\n";
    request_stream << "Accept: */*\r\n";
    request_stream << "Connection: close\r\n\r\n";

    con->write(request);

    HTTPServerResponse res = readResponse(con.get()) ;

    return res ;
}

HTTPServerResponse HTTPClientImpl::post(const URL &url, const std::map<std::string, std::string> &data) {

    std::unique_ptr<ConnectionBase> con(connect(url)) ;

    string payload ;
    for( const auto &lp: data ) {
        if ( !payload.empty() ) payload += '&' ;
        payload += lp.first + '=' + lp.second ;
    }

    asio::streambuf request;
    std::ostream request_stream(&request);

    request_stream << "POST " << url.file() << " HTTP/1.1\r\n";
    request_stream << "Host: " << url.host() << "\r\n";
    request_stream << "Content-Length:" << payload.length() << "\r\n" ;
    request_stream << "Content-Type: application/x-www-form-urlencoded\r\n" ;
    request_stream << "Accept: */*\r\n";
    request_stream << "Connection: close\r\n\r\n";

    request_stream << payload ;
    con->write(request);

    HTTPServerResponse res = readResponse(con.get()) ;

    return res ;
}


HTTPClient::HTTPClient(): impl_(new HTTPClientImpl()) {

}

HTTPClient::~HTTPClient()
{

}

HTTPServerResponse HTTPClient::get(const string &url)
{
    assert(impl_) ;
    return impl_->get(url) ;
}

HTTPServerResponse HTTPClient::post(const string &url, const std::map<string, string> &data)
{
    assert(impl_) ;
    return impl_->post(url, data) ;
}

void HTTPClient::setHost(const string &hostname)
{
    assert(impl_) ;
    impl_->host_name_ = hostname ;

}


}
