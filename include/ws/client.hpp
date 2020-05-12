#ifndef WS_HTTP_CLIENT_HPP
#define WS_HTTP_CLIENT_HPP

#include <memory>

#include <ws/url.hpp>
#include <ws/response.hpp>

namespace ws {

class HttpClientImpl ;

// Class to make blocking http requests

class HttpClient {
public:
    HttpClient() ;
   ~HttpClient() ;

    Response get(const std::string &url) ;
    Response post(const std::string &url, const std::map<std::string, std::string> &data ) ;

    void setHost(const std::string &hostname) ;

private:

    std::unique_ptr<HttpClientImpl> impl_ ;
};

class HttpClientError: public std::runtime_error {
public:
    HttpClientError(const std::string &msg): std::runtime_error(msg) {}
};

}


#endif
