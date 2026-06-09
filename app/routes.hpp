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
    Routes(xdb::Connection &con): con_(con) {}

    uint64_t createRoute(const std::string &title, const std::string &difficulty, const GPX &gpx) ;
    std::optional<Route> fetchRoute(uint64_t id) ;

private:

    void createTables() ;

    xdb::Connection &con_ ;
};