#pragma once

#include <xdb/connection.hpp>
#include <optional>
#include "util/gpx.hpp"
#include <variant/variant.hpp>
struct Route {
    uint64_t id_ ;
    std::string title_, difficulty_ ;
};

class Routes {
public:
    Routes(xdb::Connection &con): con_(con) {}

    uint64_t createRoute(const Variant &title, const std::string &difficulty, const GPX &gpx) ;
    std::optional<Route> fetchRoute(uint64_t id) ;

     void createTables() ;
private:

   

    xdb::Connection &con_ ;
};