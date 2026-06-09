#pragma once

#include <xdb/connection.hpp>
#include <optional>


struct Route {
    uint64_t id_ ;
    std::string title_ ;
    int difficulty_ ;
};

class Routes {
public:
    Routes(xdb::Connection &con): con_(con) {}

    uint64_t addRoute(const std::string &title, int difficulty) ;
    std::optional<Route> fetchRoute(uint64_t id) ;

private:

    void createTables() ;

    xdb::Connection &con_ ;
};