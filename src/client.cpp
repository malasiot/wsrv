#include <ws/client.hpp>

#include <asio/io_service.hpp>
#include <asio/connect.hpp>
#include <asio/write.hpp>
#include <asio/read_until.hpp>
#include <asio/streambuf.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/read.hpp>

#include <iostream>

using namespace std ;
using namespace asio ;
using namespace asio::ip ;

namespace ws {

class HttpClientImpl {
public:
    HttpClientImpl() = default ;

    bool get(const Url &url) ;

private:

    friend class HttpClient ;

    asio::io_service io_service_ ;
    string host_name_ ;

};

bool HttpClientImpl::get(const Url &url) {

    try {
        // Get a list of endpoints corresponding to the server name.
        tcp::resolver resolver(io_service_);
        tcp::resolver::query query(url.host(), url.schema());
        tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

        // Try each endpoint until we successfully establish a connection.
        tcp::socket socket(io_service_);
        asio::connect(socket, endpoint_iterator);

        // Form the request. We specify the "Connection: close" header so that the
        // server will close the socket after transmitting the response. This will
        // allow us to treat all data up until the EOF as the content.
        asio::streambuf request;
        std::ostream request_stream(&request);
        request_stream << "GET " << url.file() << " HTTP/1.1\r\n";
        request_stream << "Host: " << url.host() << "\r\n";
        request_stream << "Accept: */*\r\n";
        request_stream << "Connection: close\r\n\r\n";

        // Send the request.
        asio::write(socket, request);

        // Read the response status line. The response streambuf will automatically
        // grow to accommodate the entire line. The growth may be limited by passing
        // a maximum size to the streambuf constructor.
        asio::streambuf response;
        asio::read_until(socket, response, "\r\n");

        // Check that response is OK.
        std::istream response_stream(&response);
        std::string http_version;
        response_stream >> http_version;
        unsigned int status_code;
        response_stream >> status_code;
        std::string status_message;
        std::getline(response_stream, status_message);

        if (!response_stream || http_version.substr(0, 5) != "HTTP/")
        {
          std::cout << "Invalid response\n";
          return 1;
        }
        if (status_code != 200)
        {
          std::cout << "Response returned with status code " << status_code << "\n";
          return 1;
        }

        // Read the response headers, which are terminated by a blank line.
        asio::read_until(socket, response, "\r\n\r\n");

        // Process the response headers.
        std::string header;
        while (std::getline(response_stream, header) && header != "\r")
          std::cout << header << "\n";
        std::cout << "\n";

        // Write whatever content we already have to output.
        if (response.size() > 0)
          std::cout << &response;

        // Read until EOF, writing data to output as we go.
        error_code error;
        while (asio::read(socket, response,
              asio::transfer_at_least(1), error))
          std::cout << &response;
        if (error != asio::error::eof)
          throw system_error(error);
      }
      catch (std::exception& e)
      {
        std::cout << "Exception: " << e.what() << "\n";
      }

    return 0 ;
}

HttpClient::HttpClient(): impl_(new HttpClientImpl()) {

}

HttpClient::~HttpClient()
{

}

bool HttpClient::get(const string &url)
{
    assert(impl_) ;
    return impl_->get(url) ;

}

void HttpClient::setHost(const string &hostname)
{
    assert(impl_) ;
    impl_->host_name_ = hostname ;

}


}
