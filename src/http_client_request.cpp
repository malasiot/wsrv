#include <wsrv/http_client_request.hpp>
#include "detail/util.hpp"

using namespace std ;
namespace ws {

HTTPClientRequest &HTTPClientRequest::setBodyURLEncoded(const std::map<std::string, std::string> &params)
{
    for( const auto &kv: params ) {
        if ( !body_.empty() ) body_ += '&' ;
        body_ += url_encode(kv.first) + '=' + url_encode(kv.second) ;
    }

    mime_ = "application/x-www-form-urlencoded" ;

    return *this ;

}

string HTTPClientRequest::methodString() const
{
    switch ( method_ ) {
    case GET:
        return "GET" ;
    case POST:
        return "POST" ;
    case DELETE:
        return "DELETE" ;
    case PATCH:
        return "PATCH";
    case PUT:
        return "PUT";
    case HEAD:
        return "HEAD";
    case OPTIONS:
        return "OPTIONS" ;
    case TRACE:
        return "TRACE" ;

    }
}


}
