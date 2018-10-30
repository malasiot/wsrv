#ifndef WS_EXCEPTIONS_HPP
#define WS_EXCEPTIONS_HPP

#include <ws/response.hpp>

namespace ws {

class HttpResponseException {
public:
    HttpResponseException(Response::Status code, const std::string &reason = std::string()):
        code_(code), reason_(reason) {}

    Response::Status code_ ;
    std::string reason_ ;
};


}

#endif
