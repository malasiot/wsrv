#pragma once

#include <xdb/connection.hpp>
#include <optional>
#include "util/gpx.hpp"

struct Route {
    uint64_t id_ ;
    std::string title_, difficulty_ ;
};

class Routes {
public:


    static uint64_t createRoute(xdb::Connection &con, const std::string &title, const std::string &difficulty, const GPX &gpx) ;
    static std::optional<Route> fetchRoute(xdb::Connection &con, uint64_t id) ;

    static void initTables(xdb::Connection &con) ;

};