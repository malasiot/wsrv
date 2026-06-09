#pragma once

#include <wsrv/middleware.hpp>
#include <wsrv/request.hpp>
#include <wsrv/response.hpp>

namespace ws {

// data to store on the request object
struct LocaleResolverData {
    LocaleResolverData(const std::string &loc): locale_(loc) {}

    const std::string &locale() const { return locale_ ; }
    std::string locale_ ;
};

// Checks request for locale information with the following priority. 
// 1. Check route if _locale attribute is defined
// 2. Check query parameters for ?lang=<locale>
// 3. Check cookies for "locale"
// 4. Check Accept-Languages header returning the languages with the highest weight
// 5. Return fallback locale

class LocaleResolver: public IMiddleware {
public:
    LocaleResolver(const std::vector<std::string> &supported, const std::string &fallback = "en_US"):
        supported_(supported), fallback_(fallback) {}

    void handle(HTTPServerRequest& req, HTTPServerResponse& res, MiddlewareContext& ctx) override;
private:

    bool isLocaleSupported(const std::string &locale_str) const;
    std::string resolve(HTTPServerRequest &req) const ;

    std::vector<std::string> supported_ ;
    std::string fallback_;
} ;


}