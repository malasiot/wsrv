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

    static uint64_t createRoute(xdb::Connection &con, const Variant &title, const std::string &difficulty, const GPX &gpx) ;
    static std::optional<Route> fetchRoute(xdb::Connection &con, uint64_t id) ;

    static void createTables(xdb::Connection &con) ;

};