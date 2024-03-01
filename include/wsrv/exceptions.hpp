#ifndef WS_EXCEPTIONS_HPP
#define WS_EXCEPTIONS_HPP

#include <wsrv/response.hpp>

namespace ws {

class HttpResponseException {
public:
    HttpResponseException(HTTPServerResponse::Status code, const std::string &reason = std::string()):
        code_(code), reason_(reason) {}

    HTTPServerResponse::Status code_ ;
    std::string reason_ ;
};


}

#endif
