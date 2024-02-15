#include <wsrv/http_client_request.hpp>
#include "detail/util.hpp"

using namespace std ;
namespace ws {

void URLEncodedRequestBody::addToContent(const std::string &key, const std::string &val) {
    if ( !content_.empty() ) content_ += '&' ;
    content_ += url_encode(key) + '=' + url_encode(val) ;

}


}
