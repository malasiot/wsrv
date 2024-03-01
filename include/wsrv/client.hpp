#ifndef WS_HTTP_CLIENT_HPP
#define WS_HTTP_CLIENT_HPP

#include <memory>

#include <wsrv/url.hpp>
#include <wsrv/response.hpp>
#include <wsrv/http_client_request.hpp>

namespace ws {

class HTTPClientImpl ;

// Class to make blocking http requests

class HTTPClientRequest ;

class HTTPClient {
public:
    HTTPClient() ;
   ~HTTPClient() ;

    HTTPServerResponse execute(HTTPClientRequest &req) ;

    HTTPServerResponse get(const std::string &url) ;
    HTTPServerResponse post(const std::string &url, const std::map<std::string, std::string> &data ) ;

    void setHost(const std::string &hostname) ;

private:

    std::unique_ptr<HTTPClientImpl> impl_ ;
};

class HTTPClientError: public std::runtime_error {
public:
    HTTPClientError(const std::string &msg): std::runtime_error(msg) {}
};

}


#endif
