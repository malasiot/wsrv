#ifndef WS_HTTP_CLIENT_HPP
#define WS_HTTP_CLIENT_HPP

#include <memory>

#include <ws/url.hpp>

namespace ws {

class HttpClientImpl ;

class HttpClient {
public:
    HttpClient() ;
   ~HttpClient() ;

    bool get(const std::string &url) ;

    void setHost(const std::string &hostname) ;

private:

    std::unique_ptr<HttpClientImpl> impl_ ;
};


}


#endif
