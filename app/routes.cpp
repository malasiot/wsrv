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

uint64_t Routes::createRoute(xdb::Connection &con, const Variant &title, const std::string &difficulty, const GPX &gpx) {
    con.execute("INSERT INTO tracks (title, difficulty, geom) VALUES ($1, $2, ST_GeomFromText($3, 4326))", title.toJSON(), difficulty, "MULTILINESTRING EMPTY");
    return con.last_insert_rowid() ;
}

void Routes::createTables(xdb::Connection &con) {
    con.execute("SET client_min_messages = WARNING;");
    con.execute("CREATE EXTENSION IF NOT EXISTS postgis;");

    con.execute(R"(CREATE TABLE IF NOT EXISTS tracks (id SERIAL PRIMARY KEY, 
        title JSONB NOT NULL, 
        "desc" JSONB,
        stats JSONB, 
        difficulty VARCHAR(20) NOT NULL,
        created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
        updated_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
        geom GEOMETRY(MultiLineString, 4326) NOT NULL))");

    con.execute("CREATE INDEX IF NOT EXISTS idx_tracks_geom ON tracks USING gist (geom)");

    con.execute("CREATE INDEX IF NOT EXISTS idx_track_title_jsonb ON tracks USING gin (title)");
    con.execute("CREATE INDEX IF NOT EXISTS idx_track_desc_jsonb ON tracks USING gin (\"desc\")");

    con.execute(R"(CREATE TABLE IF NOT EXISTS waypoints (id SERIAL PRIMARY KEY, name JSONB NOT NULL, "desc" JSONB,
        geom GEOMETRY(Point, 4326)))");

    con.execute("CREATE INDEX IF NOT EXISTS idx_wpts_geom ON waypoints USING gist (geom)");
    con.execute("CREATE INDEX IF NOT EXISTS idx_wpts_name_jsonb ON waypoints USING gin (name)");

    con.execute(R"(CREATE TABLE IF NOT EXISTS photos (id SERIAL PRIMARY KEY, 
        url TEXT NOT NULL, 
        caption JSONB,
        track_id INTEGER NOT NULL, 
        wpt INTEGER DEFAULT NULL,
        geom GEOMETRY(Point, 4326) DEFAULT NULL,
        created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,

        CONSTRAINT fk_photo_track FOREIGN KEY (track_id) REFERENCES tracks(id) ON DELETE CASCADE
    )
        )");

    con.execute("CREATE INDEX IF NOT EXISTS idx_photos_track_id ON photos(track_id)") ;

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

