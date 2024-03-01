#pragma once

#include <wsrv/url.hpp>
#include <memory>

namespace ws {

class HTTPClientRequestBody {
public:

    virtual size_t contentLength() const = 0 ;
    virtual std::string contentType() const = 0 ;

protected:
    virtual void write(std::ostream &strm) const = 0 ;

    HTTPClientRequestBody() = default ;
};

class URLEncodedRequestBody: public HTTPClientRequestBody {
public:
    URLEncodedRequestBody() = default ;
    URLEncodedRequestBody(const std::map<std::string, std::string> &params): HTTPClientRequestBody() {
        addToContent(params) ;
    }

    URLEncodedRequestBody &addParam(const std::string &key, const std::string &val) { addToContent(key, val); return *this ;}

    size_t contentLength() const override { return content_.length() ; }
    std::string contentType() const override { return "application/x-www-form-urlencoded" ; }

protected:
    void write(std::ostream &strm) ;

private:

    void addToContent(const std::map<std::string, std::string> &params) ;
    void addToContent(const std::string &key, const std::string &val) ;

    std::string content_ ;
};

class HTTPClientRequest {
public:

    enum Method {
        GET, POST, PUT, DELETE, PATCH, HEAD, OPTIONS, TRACE
    } ;


    HTTPClientRequest(const std::string &url): url_(url) {}
    HTTPClientRequest(const URL &url): url_(url.str()) {}
    HTTPClientRequest() = default ;

    HTTPClientRequest &setURL(const std::string &url) { url_ = url ; return *this ; }
    HTTPClientRequest &setURL(const URL &url) { url_ = url.str() ; return *this ; }

    HTTPClientRequest &setHeader(const std::string &key, const std::string &val) {
        headers_.emplace(key, val) ;
        return *this ;
    }

    HTTPClientRequest &setMethod(Method m) {
        method_ = m ; return *this ;
    }

    HTTPClientRequest &setBody(HTTPClientRequestBody *b) {
        body_.reset(b) ; return *this ;
    }


    const std::string &url() const { return url_; }
    std::string header(const std::string &key) ;
    Method method() const { return method_ ; }

private:

    std::string url_ ;
    std::map<std::string, std::string> headers_ ;
    Method method_ ;
    std::unique_ptr<HTTPClientRequestBody> body_ ;
};


}
