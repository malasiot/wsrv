#include "routes.hpp"

uint64_t Routes::createRoute(const std::string &title, const std::string &difficulty, const GPX &gpx) {
    createTables() ;
    con_.execute("INSERT INTO routes (title, difficulty, geom) VALUES ($1, $2, ST_GeomFromText($3, 4326))", title, difficulty, );
    return con_.last_insert_rowid() ;
}

void Routes::createTables() {
    con_.execute(R"(CREATE TABLE IF NOT EXISTS routes (id SERIAL PRIMARY KEY, title TEXT, 
        difficulty VARCHAR(20), length FLOAT, duration FLOAT, geom GEOMTRY)");
}

std::optional<Route> Routes::fetchRoute(uint64_t id) {
    auto q = con_.query("SELECT title, difficulty FROM routes WHERE id = ? LIMIT 1", id);
    auto row = q.getOne() ;
    if ( row ) {
        Route r ;
        r.id_ = id ;
        row >> r.title_ >> r.difficulty_ ;
        return r ;
    } else 
    return {} ;
}