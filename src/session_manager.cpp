#include <wsrv/session_manager.hpp>
#include <wsrv/crypto.hpp>

#include <algorithm>

using namespace std ;

namespace ws {

string SessionManager::generateSID() {
    string code = encodeBase64(randomBytes(32)) ;
    std::replace_if(code.begin(), code.end(), [&](char c) {
        return !std::isalnum(c) ;
    }, '-') ;
    return code ;
}

} // namespace ws
