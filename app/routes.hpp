#pragma once

#include <xdb/connection.hpp>
#include <optional>
#include "util/gpx.hpp"
#include <variant/variant.hpp>

struct RouteEntry {
    uint64_t id_ ;
    std::string title_, difficulty_ ;
};

struct PhotoImageData {
    int64_t id_ ;
    std::string data_ ;
    std::string mime_ ;
};

struct Photo {
    int64_t id_ ;
    std::string caption_ ;
};

class Routes {
public:

    static int64_t createRoute(xdb::Connection &con, const Variant &title, const std::string &difficulty, const GPX &gpx) ;
    static std::optional<RouteEntry> fetchRoute(xdb::Connection &con, uint64_t id) ;
    static void updateRoute(xdb::Connection &con, int64_t id, const std::string &title,
        const std::string &desc, float duration, float length, const std::string &difficulty, const std::string &locale) ;

    static int64_t addPhoto(xdb::Connection &con, uint64_t track_id, const std::string &data, 
        const std::string &mime, int64_t wpt_id) ;
    static std::optional<PhotoImageData> getPhoto(xdb::Connection &con, uint64_t id) ;
    static std::vector<Photo> getAllPhotosForTrack(xdb::Connection &con, int64_t track_id, const std::string &loc) ;
    static std::vector<Photo> getAllPhotosForWaypoint(xdb::Connection &con, int64_t wpt_id, const std::string &loc) ;
    static void deletePhoto(xdb::Connection &con, int64_t id);
    static void updatePhotoCaption(xdb::Connection &con, int64_t id, const std::string &cap, const std::string &locale) ;


    static void createTables(xdb::Connection &con) ;
    
};