#include <ws/request.hpp>
#include <ws/route.hpp>
#include "detail/connection.hpp"

using namespace std ;

namespace ws {

bool Request::matches(const string &method, const string &pattern, Dictionary &attributes) const
{
    return matchesMethod(method) && Route(pattern).matches(path_, attributes) ;
}

bool Request::matches(const string &method, const string &pattern) const
{
    Dictionary attributes ;
    return matchesMethod(method) && Route(pattern).matches(path_, attributes) ;
}

Session &Request::getSession() const {
    return ctx_->getSession() ;
}

string Request::toString() const
{
   ostringstream strm ;
    strm <<  getServerAttribute("REMOTE_ADDR", "127.0.0.1")
             << ": \"" << method_ << " " << path_
             << ((query_.empty()) ? "" : "?" + query_) << " "
             << protocol_ << "\"";

    return strm.str() ;
}

Request::Request(HttpConnection *ctx): ctx_(ctx) {

}

bool Request::supportsGzip() const {
    string enc = getServerAttribute("Accept-Encoding") ;
    return !enc.empty() && enc.find("gzip") != string::npos ;
}

string Request::getServerAttribute(const string &key, const string &def) const {
    auto it = SERVER_.find(key) ;
    return ( it == SERVER_.end() ) ? def : it->second ;
}

string Request::getQueryAttribute(const string &key, const string &def) const {
    auto it = GET_.find(key) ;
    return ( it == GET_.end() ) ? def : it->second ;
}

string Request::getPostAttribute(const string &key, const string &def) const {
    auto it = POST_.find(key) ;
    return ( it == POST_.end() ) ? def : it->second ;
}

string Request::getCookie(const string &key, const string &def) const {
    auto it = COOKIE_.find(key) ;
    return ( it == COOKIE_.end() ) ? def : it->second ;
}

bool Request::matches(const string &method, const Route &pattern, Dictionary &attributes) const
{
    return matchesMethod(method) && pattern.matches(path_, attributes) ;
}

bool Request::matches(const string &method, const Route &pattern) const
{
    Dictionary attributes ;
    return matchesMethod(method) && pattern.matches(path_, attributes) ;
}

bool Request::matchesMethod(const string &method) const {
    vector<string> methods ;

    static std::regex regex{R"([\s|]+)"}; // split on space and caret
    std::sregex_token_iterator it{method.begin(), method.end(), regex, -1};
    methods.assign(it, {}) ;

    return std::find(methods.begin(), methods.end(), method_) != methods.end() ;
}

} // namespace ws
