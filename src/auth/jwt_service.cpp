#include <wsrv/auth/jwt_service.hpp>
#include <wsrv/auth/authenticator.hpp>
#include <jwt-cpp/jwt.h>
namespace ws {

    // Generates a token containing only the agnostic identifier
    std::string JWTService::generateToken(const std::shared_ptr<IAuthenticatedUser>& user, int expiry_seconds) {
        auto now = std::chrono::system_clock::now();
            
        return jwt::create()
            .set_issuer(issuer_)
            .set_issued_at(now)
            .set_expires_at(now + std::chrono::seconds(expiry_seconds))
            .set_subject(user->getUniqueId()) 
            .sign(jwt::algorithm::hs256{secret_key_});
    }

        // Verifies the token and extracts the subject (User ID). Returns empty string if invalid.
    std::string JWTService::verifyToken(const std::string& token) {
        try {
            auto decoded = jwt::decode(token);
                
            // Set up the verification constraints
            auto verifier = jwt::verify()
                    .allow_algorithm(jwt::algorithm::hs256{secret_key_})
                    .with_issuer(issuer_);

            verifier.verify(decoded);
                
                // Return the agnostic user identifier stored in the subject field
             return decoded.get_subject();
        } 
        catch (const std::exception& e) {
            // Token was tampered with, expired, or signature didn't match
            return "";
        }
    }
};

