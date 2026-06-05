#include <wsrv/middleware/locale.hpp>
#include <algorithm>

using namespace std ;

namespace ws {
bool LocaleResolver::isLocaleSupported(const std::string &locale_str) const {
    for( const auto &lang: supported_ ) {
        if ( lang == locale_str ) return true ;
        if ( locale_str.size() >= 2 && lang.substr(0, 2) == locale_str.substr(0, 2) ) return true ;
    }
    return false ;
}

void LocaleResolver::handle(HTTPServerRequest& req, HTTPServerResponse& res, MiddlewareContext& ctx) {
    string resolved = resolve(req) ;
    req.data().set(std::make_shared<LocaleResolverData>(resolved));
    ctx.next(req, res) ;
}

struct LanguagePreference {
    std::string code;
    double weight;
};

static std::vector<LanguagePreference> parse_accept_language(const std::string& header) {
    std::vector<LanguagePreference> preferences;
    std::stringstream ss(header);
    std::string item;

    // parse command separated language definitions
    while (std::getline(ss, item, ',')) {
        // Remove spaces
        item.erase(std::remove_if(item.begin(), item.end(), ::isspace), item.end());
        if (item.empty()) continue;

        size_t q_pos = item.find(";q=");
        std::string lang = item.substr(0, q_pos);
        double weight = 1.0; // Default weight

        if ( q_pos != std::string::npos ) {
            try {
                weight = std::stod(item.substr(q_pos + 3));
            } catch (...) {
                weight = 0.0; // Fallback for malformed q-values
            }
        }
        preferences.push_back({lang, weight});
    }

    // Sort preferences descending by weight
    std::sort(preferences.begin(), preferences.end(), [](const LanguagePreference& a, const LanguagePreference& b) {
        return a.weight > b.weight;
    });

    return preferences;
}

std::string LocaleResolver::resolve(HTTPServerRequest &req) const {
    string resolved = req.getRouteAttribute("_locale") ;
    if ( !resolved.empty() && isLocaleSupported(resolved)) return resolved ;

    resolved = req.getQueryAttribute("lang") ;
    if ( !resolved.empty() && isLocaleSupported(resolved)) return resolved ;

    resolved = req.getCookie("locale") ;
    if ( !resolved.empty() && isLocaleSupported(resolved)) return resolved ;

    // check header for Accept-Language

    std::string accept_lang_header = req.getServerAttribute("Accept-Language");
    if ( !accept_lang_header.empty() ) {
        auto languages = parse_accept_language(accept_lang_header) ;

        for ( const auto &lang: languages ) { // iterating from most desirable to less desirable
            if ( isLocaleSupported(lang.code) ) return lang.code ;
        }
    }

    // nothing matched
    return fallback_ ;
}
    

}