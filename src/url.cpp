#include <ws/url.hpp>
#include "detail/util.hpp"
#include <algorithm>

using namespace std ;

namespace ws {

string Url::file() const
{
    string res = path_ ;
    if ( !query_.empty() ) {
        res.push_back('?') ;
        res.append(query_) ;
    }
    if ( !fragment_.empty() ) {
        res.push_back('#');
        res.append(fragment_) ;
    }

    return res ;
}

string Url::str() const
{
    string res ;

    res += schema_ ;
    res += "://" ;
    res += host_ ;
    if ( !port_.empty() )  {
        res.push_back(':') ;
        res.append(port_) ;
    }
    res += file();

    return res ;
}

void Url::parse(const std::string &url)
{
    size_t cursor = 0, pos = url.find("://") ;

    if ( pos == string::npos )
        return ;

    schema_ = url.substr(cursor, pos) ;

    cursor = pos + 3 ;

    pos = url.find('/', cursor) ;

    if ( pos != string::npos ) {
        auto tokens = split(url.substr(cursor, pos - cursor), ":") ;
        host_ = tokens[0] ;
        if ( tokens.size() > 1 )
            port_ = tokens[1] ;
        cursor = pos ;
    } else
        return ;

    pos = url.find('?', cursor) ;

    if ( pos == string::npos )
        path_ = url.substr(cursor) ;
    else {
        path_ = url.substr(cursor, pos - cursor) ;
        query_ = url.substr(pos+1);
    }
}
}
