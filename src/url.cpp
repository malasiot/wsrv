#include <wsrv/url.hpp>
#include "detail/util.hpp"
#include <algorithm>

using namespace std ;

namespace ws {

string URL::file(bool encoded) const
{

    string res = makePathString(encoded) ;
    string query = makeQueryString(encoded) ;
    if ( !query.empty() ) {
        res.push_back('?') ;
        res.append(query) ;
    }
    if ( !fragment_.empty() ) {
        res.push_back('#');
        res.append(encoded ? url_encode(fragment_) : fragment_) ;
    }

    return res ;
}

string URL::str(bool encoded) const
{
    stringstream res ;

    if ( !protocol_.empty() )
        res << protocol_ << "://" ;
    res << host_ ;
    if ( port_ > 0 )  {
        res << ':' << port_ ;
    }
    res << '/' << file(encoded);

    return res.str() ;
}

bool URL::hasQueryParam(const std::string &p) const {
    return params_.count(p) != 0 ;
}

string URL::queryParam(const std::string &p, const std::string &def) const {
    auto it = params_.find(p) ;
    return ( it == params_.end() ) ? def : it->second ;
}

URL & URL::normalizePath()
{
    if ( segments_.empty() ) return *this;

    std::vector<std::string> dst_segments;

    for( const string &seg: segments_ ) {
        if ( seg.empty() ) continue ;
        else if ( seg == "." ) continue ;
        else if ( seg == "..") {
            if ( !dst_segments.empty() )
                dst_segments.pop_back() ;
        }
        else {
            dst_segments.push_back(seg) ;
        }
    }

    segments_ = dst_segments ;

    return *this ;

}


void URL::parse(const std::string &url)
{
    size_t cursor = 0, pos = url.find("://") ;

    if ( pos != string::npos ) {
        protocol_ = url.substr(cursor, pos) ;
        cursor = pos + 3 ;
    }
    pos = url.find('/', cursor) ;

    auto tokens = split(url.substr(cursor, ( pos == string::npos ) ?  pos : pos - cursor), ":") ;
    host_ = tokens[0] ;
    if ( tokens.size() > 1 )
        port_ = stoi(tokens[1]) ;
    cursor = pos ;

    if ( cursor == string::npos )
        return ;
    else
        cursor ++ ;

    pos = url.find('?', cursor) ;

    if ( pos == string::npos )
        parsePath(url.substr(cursor)) ;
    else {
        parsePath(url.substr(cursor, pos - cursor)) ;
        string query = url.substr(pos+1);
        for( const auto &t: split(query, "&") ) {
            auto kv = split(t, "=") ;
            if ( kv.size() == 2 ) {
                params_.emplace(url_decode(kv[0]), url_decode(kv[1])) ;
            }
        }
    }
}

void URL::parsePath(const std::string &p) {
    if ( p.empty() ) return ;
    auto tokens = split(p, "/") ;
    for(const auto &s: tokens )
        if ( !s.empty() )
            segments_.emplace_back(url_decode(s)) ;
}

string URL::makeQueryString(bool encoded) const {
    stringstream res ;
    bool first = true ;
    for( const auto &kv: params_ ) {
        if ( !first )
            res << '&' ;
        string key = encoded ? url_encode(kv.first) : kv.first;
        string val = encoded ? url_encode(kv.second) : kv.second ;

        res << key << '=' << val ;
        first = false ;
    }
    return res.str() ;
}

string URL::makePathString(bool encoded) const {
    stringstream res ;
    bool first = true ;
    for( const auto &s: segments_ ) {
        if ( !first )
            res << '/' ;
        string seg = encoded ? url_encode(s) : s ;

        res << seg ;
        first = false ;
    }

    return res.str() ;
}


URLBuilder::URLBuilder(const URL &uri):
    protocol_(uri.protocol()), host_(uri.host()), port_(uri.port()), segments_(uri.pathSegments()), params_(uri.queryParams()), fragment_(uri.fragment()) {
}

URL URLBuilder::build() const {
    URL url ;
    url.host_ = host_ ;
    url.port_ = port_ ;
    url.protocol_ = protocol_ ;
    url.segments_ = segments_ ;
    url.params_ = params_ ;
    url.fragment_ = fragment_ ;
    return url ;
}

void URLBuilder::parsePath(const std::string &p) {
    auto tokens = split(p, "/") ;
    for( const auto &t: tokens ) {
        if ( !t.empty() ) segments_.push_back(t) ;
    }
}

URLBuilder::URLBuilder(const std::string &str): URLBuilder(URL(str)) {

}

}
