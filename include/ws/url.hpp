#ifndef WS_URL_HPP
#define WS_URL_HPP

#include <string>
#include <map>

namespace ws {

// simple Url parser class. Url parts should be properly encoded

class Url {
public:

    Url(const std::string &url) { parse(url) ; }

    const std::string &host() const { return host_ ; }
    const std::string &port() const { return port_ ; }
    const std::string &path() const { return path_ ; }
    std::string file() const ;
    const std::string &schema() const { return schema_ ; }

    std::string str() const ;

protected:

    void parse(const std::string &url) ;

    std::string schema_, host_, port_, fragment_, path_, query_ ;
    std::map<std::string, std::string> params_ ;
};


}
#endif
