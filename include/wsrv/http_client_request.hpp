#pragma once

#include <wsrv/url.hpp>
#include <memory>

namespace ws {

class HTTPClientRequest {
public:

    enum Method {
        GET, POST, PUT, DELETE, PATCH, HEAD, OPTIONS, TRACE
    } ;


    HTTPClientRequest(const std::string &url): url_(url) {}
    HTTPClientRequest(const URL &url): url_(url) {}
    HTTPClientRequest() = delete ;


    HTTPClientRequest &setURL(const std::string &url) { url_ = URL(url) ; return *this ; }
    HTTPClientRequest &setURL(const URL &url) { url_ = url ; return *this ; }

    HTTPClientRequest &setHeader(const std::string &key, const std::string &val) {
        headers_.emplace(key, val) ;
        return *this ;
    }

    HTTPClientRequest &setMethod(Method m) {
        method_ = m ; return *this ;
    }

    HTTPClientRequest &setBodyURLEncoded(const std::map<std::string, std::string> &params) ;
    HTTPClientRequest &setBody(const std::string &contentType, const std::string &content) {
        body_ = content ;
        mime_ = contentType ;
        return *this ;
    }


    const URL &url() const { return url_; }
    std::string header(const std::string &key) ;
    const std::map<std::string, std::string> &headers() const { return headers_ ; }
    Method method() const { return method_ ; }
    std::string methodString() const ;
    const std::string &body() const { return body_ ; }
    const std::string &contentType() const { return mime_ ; }

private:


    URL url_ ;
    std::map<std::string, std::string> headers_ ;
    Method method_ = GET ;
    std::string body_ ;
    std::string mime_ ;
};


}
