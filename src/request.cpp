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
    strm <<  SERVER_.get("REMOTE_ADDR", "127.0.0.1")
             << ": \"" << method_ << " " << path_
             << ((query_.empty()) ? "" : "?" + query_) << " "
             << protocol_ << "\"";

    return strm.str() ;
}

Request::Request(HttpConnection *ctx): ctx_(ctx) {

}

bool Request::supportsGzip() const {
    string enc = SERVER_.get("Accept-Encoding") ;
    return !enc.empty() && enc.find("gzip") != string::npos ;
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
