#pragma once

#include <string>
#include <memory>

namespace ws {

class IAuthenticatedUser ;
class JWTService {
private:
    std::string secret_key_;
    std::string issuer_;

public:
    JWTService(std::string secret, std::string issuer = "wsrv")
        : secret_key_(std::move(secret)), issuer_(std::move(issuer)) {}

    // Generates a token containing only the agnostic identifier
    std::string generateToken(const std::shared_ptr<IAuthenticatedUser>& user, int expiry_seconds = 3600) ;
        // Verifies the token and extracts the subject (User ID). Returns empty string if invalid.
    std::string verifyToken(const std::string& token) ; 
};
}