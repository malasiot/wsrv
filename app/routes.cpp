#include "routes.hpp"

uint64_t Routes::addRoute(const std::string &title, int difficulty) {
    createTables() ;
    con_.execute("INSERT INTO routes (title, difficulty) VALUES (?, ?)", title, difficulty);
    return con_.last_insert_rowid() ;
}

void Routes::createTables() {
    con_.execute("CREATE TABLE IF NOT EXISTS routes (id INTEGER PRIMARY KEY, title TEXT, difficulty INTEGER)");
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