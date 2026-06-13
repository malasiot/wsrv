#include "routes.hpp"

#include <iostream>

using namespace std ;

static string gpxToWKTLines(const GPX &data) {
    stringstream ls ;
    ls << "MULTILINESTRING(" ;
        
    bool is_first_seg = true ;
    for( const auto &trk: data.tracks_ ) {
       
        for(const auto &seg: trk.segments_ ) {
            if ( seg.points_.size() < 2 ) continue ;
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

    ofstream ss("/tmp/test.txt") ;
    ss << ls.str() ;
    return ls.str() ;
}

int64_t Routes::createRoute(xdb::Connection &con, const std::string &title, const std::vector<std::string> &loc, const GPX &gpx) {
    Variant::Object loc_title ;
    for( const auto &locale: loc )
        loc_title.emplace(locale, title) ;
    int64_t track_id = con.execInsert("INSERT INTO tracks (title, geom) VALUES ($1::json, ST_GeomFromText($2, 4326)) RETURNING id", Variant(loc_title).toJSON(), gpxToWKTLines(gpx));
    xdb::Transaction trans(con) ;
    auto stmt = con.prepareStatement("INSERT INTO waypoints (name, ele, geom, track_id) VALUES ($1::json, $2, ST_SetSRID(ST_MakePoint($3, $4), 4326), $5 )");

    for( const auto &wp: gpx.waypoints_ ) {
        stmt.clear() ;
        Variant::Object loc_name ;
        for( const auto &locale: loc )
            loc_name.emplace(locale, wp.name_) ;
        stmt.bindAll(Variant(loc_name).toJSON(), wp.ele_, wp.lat_, wp.lon_, track_id) ;
        stmt.exec() ;
    }

    trans.commit() ;
    return track_id ;

}

int64_t Routes::addPhoto(xdb::Connection &con, uint64_t track_id, const std::string &data,
const std::string &mime, int64_t wpt_id) {
    xdb::Blob img(data.data(), data.data() + data.size()) ;
    std::optional<int64_t> wpt ;
    if ( wpt_id != -1 ) wpt = wpt_id ;
    return con.execInsert("INSERT INTO photos (data, mime, track_id, wpt) VALUES ($1, $2, $3, $4) RETURNING id", img, mime, track_id, wpt_id);
}


void Routes::createTables(xdb::Connection &con) {
    con.execute("SET client_min_messages = WARNING;");
    con.execute("CREATE EXTENSION IF NOT EXISTS postgis;");

    con.execute(R"(CREATE TABLE IF NOT EXISTS tracks (id SERIAL PRIMARY KEY, 
        title JSONB NOT NULL , 
        "desc" JSONB DEFAULT '{}'::jsonb,
        difficulty VARCHAR(20) DEFAULT 'easy',
        length DOUBLE PRECISION DEFAULT 0,
        duration DOUBLE PRECISION DEFAULT 0,
        created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
        updated_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
        geom GEOMETRY(MultiLineString, 4326) NOT NULL))");

    con.execute("CREATE INDEX IF NOT EXISTS idx_tracks_geom ON tracks USING gist (geom)");

    con.execute("CREATE INDEX IF NOT EXISTS idx_track_title_jsonb ON tracks USING gin (title)");
    con.execute("CREATE INDEX IF NOT EXISTS idx_track_desc_jsonb ON tracks USING gin (\"desc\")");

    con.execute(R"(CREATE TABLE IF NOT EXISTS waypoints (id SERIAL PRIMARY KEY, name JSONB NOT NULL, "desc" JSONB DEFAULT '{}'::jsonb,
        ele DOUBLE PRECISION DEFAULT NULL, track_id INTEGER NOT NULL, geom GEOMETRY(Point, 4326) ,
         CONSTRAINT fk_wpt_track FOREIGN KEY (track_id) REFERENCES tracks(id) ON DELETE CASCADE) )");

    con.execute("CREATE INDEX IF NOT EXISTS idx_wpts_geom ON waypoints USING gist (geom)");
    con.execute("CREATE INDEX IF NOT EXISTS idx_wpts_name_jsonb ON waypoints USING gin (name)");
    con.execute("CREATE INDEX IF NOT EXISTS idx_wpts_track_id ON waypoints(track_id)") ;

    con.execute(R"(CREATE TABLE IF NOT EXISTS photos (id SERIAL PRIMARY KEY, 
        data BYTEA NOT NULL,
        mime TEXT NOT NULL, 
        caption JSONB DEFAULT '{}'::jsonb,
        track_id INTEGER NOT NULL, 
        wpt INTEGER DEFAULT NULL,
        geom GEOMETRY(Point, 4326) DEFAULT NULL,
        created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,

        CONSTRAINT fk_photo_track FOREIGN KEY (track_id) REFERENCES tracks(id) ON DELETE CASCADE
    )
        )");

    con.execute("CREATE INDEX IF NOT EXISTS idx_photos_track_id ON photos(track_id)") ;

}

std::optional<PhotoImageData> Routes::getPhoto(xdb::Connection &con, uint64_t id) {
    auto q = con.query("SELECT data, mime FROM photos WHERE id = $1 LIMIT 1", id);
    auto row = q.getOne() ;

    xdb::Blob img ;
    if ( row ) {
        PhotoImageData photo ;
        photo.id_ = id ;
        
        row >> img >> photo.mime_ ;
        photo.data_.assign(img.data(), img.data() + img.size()) ;
        return photo ;
    } else 
    return {} ;
}

std::vector<Photo> Routes::getAllPhotosForTrack(xdb::Connection &con, int64_t track_id, const std::string &locale) {
    vector<Photo> ids ;
    string sql = "SELECT id, caption->>'" + locale + "' FROM photos WHERE track_id = $1 AND wpt = -1" ;
    auto q = con.query(sql.c_str(), track_id);
    for ( auto row: q ) {
       Photo p ;
       row.into(p.id_, p.caption_) ;

       ids.push_back(std::move(p)) ;
    } 
    return ids ;
}

std::vector<Photo> Routes::getAllPhotosForWaypoint(xdb::Connection &con, int64_t wpt_id, const std::string &locale) {
    vector<Photo> ids ;
    string sql = "SELECT id, caption->>'" + locale + "' FROM photos WHERE wpt = $1" ;
    auto q = con.query(sql.c_str(), wpt_id);
    for ( auto row: q ) {
       Photo p ;
       row.into(p.id_, p.caption_) ;

       ids.push_back(std::move(p)) ;
    } 
    return ids ;
}

std::vector<WaypointInfo> Routes::getAllWaypoints(xdb::Connection &con, int64_t track_id, const std::string &loc) {
    vector<WaypointInfo> data ;
    string sql = "SELECT id, name->>$1, \"desc\"->>$1  FROM waypoints WHERE track_id = $2 ORDER BY id" ;
    auto q = con.query(sql.c_str(), loc, track_id);
    for ( auto row: q ) {
       WaypointInfo info ;
       row.into(info.id_, info.name_) ;

       data.push_back(std::move(info)) ;
    } 
    return data ;
}

void Routes::updateWaypoint(xdb::Connection &con, int64_t wpt_id,
        const std::string &name, const std::string &desc, const std::string &loc) {
con.execute(R"(UPDATE waypoints SET name = jsonb_set(name, ARRAY[$1]::text[], to_jsonb($2::text)), 
    "desc" = jsonb_set("desc", ARRAY[$1]::text[], to_jsonb($3::text)) WHERE id=$4 )", loc, name, desc, wpt_id);

}
void Routes::deleteWaypoint(xdb::Connection &con, int64_t wpt_id) {
    con.execute("DELETE FROM waypoints WHERE id = $1", wpt_id);
}

void Routes::deletePhoto(xdb::Connection &con, int64_t photo_id) {
    con.execute("DELETE FROM photos WHERE id = $1", photo_id);
}

void Routes::updatePhotoCaption(xdb::Connection &con, int64_t id, const std::string &cap, const std::string &locale) {
    string sql = "UPDATE photos SET caption = jsonb_set( COALESCE(caption, '{}'::jsonb), '{" + locale + "}', to_jsonb($1::text)) WHERE id = $2";
    con.execute(sql.c_str(), cap, id);
}

std::optional<RouteEntry> Routes::fetchRoute(xdb::Connection &con, uint64_t id, const std::string &loc) {
    auto q = con.query("SELECT title->>$1 as t, \"desc\"->>$1 as d, difficulty, length, duration FROM tracks WHERE id = $2 LIMIT 1", loc, id);
    auto row = q.getOne() ;
    if ( row ) {
        RouteEntry r ;
        r.id_ = id ;
        row >> r.title_ >> r.desc_ >> r.difficulty_ >> r.length_ >> r.duration_  ;
        return r ;
    } else 
    return {} ;
}

void Routes::updateRoute(xdb::Connection &con, int64_t id, const std::string &title,
        const std::string &desc, float duration, float length, const std::string &difficulty, const std::string &locale) {
    string sql = "UPDATE tracks SET title = jsonb_set(title, '{" + locale + "}', to_jsonb($1::text)) " ;
    sql += ", \"desc\" = jsonb_set(\"desc\", '{" + locale + "}', to_jsonb($2::text))" ;
    sql += ", length = $3, duration = $4 WHERE id = $5" ;

    con.execute(sql.c_str(), title, desc, length, duration, id);
}


std::vector<RouteEntry> Routes::getAllRoutes(xdb::Connection &con, const std::string &loc) {
    vector<RouteEntry> data ;
    string sql = "SELECT id, title->>$1, length, duration, difficulty FROM tracks ORDER BY id" ;
    auto q = con.query(sql.c_str(), loc);
    for ( auto row: q ) {
       RouteEntry info ;
       row >> info.id_ >> info.title_ >> info.length_ >> info.duration_ >> info.difficulty_ ;

       data.push_back(std::move(info)) ;
    } 
    return data ;
}