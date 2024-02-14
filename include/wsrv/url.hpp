#ifndef WS_URL_HPP
#define WS_URL_HPP

#include <string>
#include <map>
#include <vector>

namespace ws {

class URLBuilder ;

// simple Url parser class. Url parts are UTF-8 decoded

class URL {
public:

    URL(const std::string &url) { parse(url) ; }

    const std::string &host() const { return host_ ; }
    int port() const { return port_ ; }
    std::string path() const { return makePathString(false) ; }
    std::string query() const { return makeQueryString(false) ;}
    const std::string &fragment() const { return fragment_ ; }

    const std::vector<std::string> &pathSegments() const { return segments_ ; }

    std::string file(bool encoded = false) const ;
    const std::string &protocol() const { return protocol_ ; }

    // creates a string representation
    std::string str(bool encoded = true) const ;

    bool hasQueryParam(const std::string &p) const ;
    std::string queryParam(const std::string &p, const std::string &def = {}) const ;
    const std::map<std::string, std::string> &queryParams() const { return params_ ; }

    URL & normalizePath() ;

protected:
    URL() = default ;

    friend class URLBuilder ;
    friend class HTTPRequest ;

    void parse(const std::string &url) ;
    void parsePath(const std::string &p) ;
    std::string makeQueryString(bool encoded) const ;
    std::string makePathString(bool encoded) const ;

    std::string protocol_, host_, fragment_, path_ ;
    int port_ = -1 ;
    std::map<std::string, std::string> params_ ;
    std::vector<std::string> segments_ ;
};

class URLBuilder {
public:
    URLBuilder(const std::string &str) ;
    URLBuilder(const URL &url) ;
    URLBuilder() = default ;

    URLBuilder &setProtocol(const std::string &pr) { protocol_ = pr ; return *this ; }
    URLBuilder &setPort(int port) { port_ = port ; return *this ; }
    URLBuilder &setHost(const std::string &host) { host_ = host ; return *this ; }
    URLBuilder &setPath(const std::string &p) { parsePath(p) ; return *this ; }
    URLBuilder &setPathSegments(const std::vector<std::string> &segs) { segments_ = segs ; return *this ; }
    URLBuilder &addPathSegment(const std::string &seg) { segments_.emplace_back(seg) ; return *this ; }

    URLBuilder &setFragment(const std::string &frag) { fragment_ = frag ; return *this ; }
    URLBuilder &addQueryParam(const std::string &key, const std::string &val) { params_.emplace(key, val) ; return *this ; }

    URL build() const ;

private:

    void parsePath(const std::string &p) ;

    std::string protocol_, host_, fragment_ ;
    std::map<std::string, std::string> params_ ;
    std::vector<std::string> segments_ ;
    int port_ = -1 ;

};

}
#endif
