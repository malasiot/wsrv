#ifndef WS_ROUTE_HPP
#define WS_ROUTE_HPP

#include <string>
#include <map>
#include <memory>

namespace ws {

struct RouteImpl ;

class Route {
    using Dictionary = std::map<std::string, std::string> ;
public:

    // Create a uri path route from the given pattern.

    Route(const std::string &pattern) ;
    ~Route() ;

    // matches the request uri to the pattern
    // pattern is the uri pattern in the form /<pat1>/<pat2>/<pat3> ... /<patn>/
    // where each sub-pattern has the format  ({[<param>][:<regex>]}|<characters>)[?]
    // e.g. /user/{id:\d+}/{action:show|hide}/
    // If the match is succesfull the method returns true and recovers the named parameters values (e.g. id, action).
    // use ? after a sub-pattern to indicate that the element (and the subsequent elements) is optional
    // Leading and trailing slashes are ignored.

    bool matches(const std::string &path, Dictionary &data) const ;
    bool matches(const std::string &path) const ;

    // generate a url from the given route replacing parameters

    std::string url(const Dictionary &params, bool relative = true) const  ;

private:

    std::unique_ptr<RouteImpl> impl_ ;
};

}

#endif
