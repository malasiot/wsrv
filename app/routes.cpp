#include "routes.hpp"

#include <iostream>

using namespace std ;

static string gpxToWKTLines(const GPX &data) {
    stringstream ls ;
    ls << "MULTILINESTRING(" ;
        
    bool is_first_seg = true ;
    for( const auto &trk: data.tracks_ ) {
       
        for(const auto &seg: trk.segments_ ) {
            if ( seg.points_.empty() ) continue ;
            if ( !is_first_seg ) ls <<  ',' ;
            else is_first_seg = false ;
            
            ls << '(' ;
           
            bool is_first_pt = true ;
            for( const auto &pt: seg.points_ ) {
                if ( !is_first_pt ) ls << ',' ;
                else is_first_pt = false ;
                
                ls << pt.lat_ << ' ' << pt.lon_ ; 
            }
            ls << ')';

        }

    }

    ls << ')' ;
    return ls.str() ;
}

uint64_t Routes::createRoute(xdb::Connection &con, const std::string &title, const std::string &difficulty, const GPX &gpx) {
    con.execute("INSERT INTO routes (title, difficulty, geometry) VALUES (?, ?, ST_GeomFromText(?, 4326))", 
        title, difficulty, gpxToWKTLines(gpx));
    return con.last_insert_rowid() ;
}


std::optional<Route> Routes::fetchRoute(xdb::Connection &con, uint64_t id) {
    auto q = con.query("SELECT title, difficulty FROM routes WHERE id = ? LIMIT 1", id);
    auto row = q.getOne() ;
    if ( row ) {
        Route r ;
        r.id_ = id ;
        row >> r.title_ >> r.difficulty_ ;
        return r ;
    } else 
    return {} ;
}

 


void Routes::initTables(xdb::Connection &con) {
    try {
        con.execute("CREATE TABLE IF NOT EXISTS routes ("
            "    id INT AUTO_INCREMENT PRIMARY KEY,"
            "    title VARCHAR(250) NOT NULL,"
            "    difficulty VARCHAR(20) NOT NULL,"
            "    geometry MULTILINESTRING NOT NULL,"
            "    SPATIAL INDEX(geometry)"
            ") ENGINE=InnoDB;");
    } catch ( xdb::Exception &e ) {
        cerr << e.what() << endl ;
    }

}