#include <wsrv/request.hpp>
#include <wsrv/route.hpp>
#include "detail/connection.hpp"

#include <regex>

using namespace std ;

namespace ws {

bool HTTPRequest::matches(const string &method, const string &pattern, Dictionary &attributes) const
{
    return matchesMethod(method) && Route(pattern).matches(path_, attributes) ;
}

bool HTTPRequest::matches(const string &method, const string &pattern) const
{
    Dictionary attributes ;
    return matchesMethod(method) && Route(pattern).matches(path_, attributes) ;
}

string HTTPRequest::toString() const
{
   ostringstream strm ;
    strm <<  getServerAttribute("REMOTE_ADDR", "127.0.0.1")
             << ": \"" << method_ << " " << path_
             << ((query_.empty()) ? "" : "?" + query_) << " "
             << protocol_ << "\"";

    return strm.str() ;
}

HTTPRequest::HTTPRequest(HttpConnection *ctx): ctx_(ctx) {

}

bool HTTPRequest::supportsGzip() const {
    string enc = getServerAttribute("Accept-Encoding") ;
    return !enc.empty() && enc.find("gzip") != string::npos ;
}

string HTTPRequest::getServerAttribute(const string &key, const string &def) const {
    auto it = SERVER_.find(key) ;
    return ( it == SERVER_.end() ) ? def : it->second ;
}

string HTTPRequest::getQueryAttribute(const string &key, const string &def) const {
    auto it = GET_.find(key) ;
    return ( it == GET_.end() ) ? def : it->second ;
}

string HTTPRequest::getPostAttribute(const string &key, const string &def) const {
    auto it = POST_.find(key) ;
    return ( it == POST_.end() ) ? def : it->second ;
}

string HTTPRequest::getCookie(const string &key, const string &def) const {
    auto it = COOKIE_.find(key) ;
    return ( it == COOKIE_.end() ) ? def : it->second ;
}

bool HTTPRequest::matches(const string &method, const Route &pattern, Dictionary &attributes) const
{
    return matchesMethod(method) && pattern.matches(path_, attributes) ;
}

bool HTTPRequest::matches(const string &method, const Route &pattern) const
{
    Dictionary attributes ;
    return matchesMethod(method) && pattern.matches(path_, attributes) ;
}

bool HTTPRequest::matchesMethod(const string &method) const {
    vector<string> methods ;

    static std::regex regex{R"([\s|]+)"}; // split on space and caret
    std::sregex_token_iterator it{method.begin(), method.end(), regex, -1};
    methods.assign(it, {}) ;

    return std::find(methods.begin(), methods.end(), method_) != methods.end() ;
}

} // namespace ws
