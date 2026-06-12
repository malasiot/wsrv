#pragma once

#include <xdb/connection.hpp>
#include <optional>
#include "util/gpx.hpp"
#include <variant/variant.hpp>
struct Route {
    uint64_t id_ ;
    std::string title_, difficulty_ ;
};

struct Photo {
    uint64_t id_ ;
    std::string data_ ;
    std::string mime_ ;
};

class Routes {
public:

    static int64_t createRoute(xdb::Connection &con, const Variant &title, const std::string &difficulty, const GPX &gpx) ;
    static std::optional<Route> fetchRoute(xdb::Connection &con, uint64_t id) ;
    static int64_t addPhoto(xdb::Connection &con, uint64_t track_id, const std::string &data, 
        const std::string &mime, int64_t wpt_id) ;
    static std::optional<Photo> getPhoto(xdb::Connection &con, uint64_t id) ;
    static void createTables(xdb::Connection &con) ;
    static std::vector<int64_t> getAllPhotos(xdb::Connection &con, int64_t id) ;
    static void deletePhoto(xdb::Connection &con, int64_t id);
    static void updatePhotoCaption(xdb::Connection &con, int64_t id, const std::string &cap, const std::string &locale) ;
};