#include <ws/fs_session_handler.hpp>

#include <iostream>
#include <chrono>
#include <random>

#include <sqlite3.h>

using namespace std ;


namespace ws {

class SQLite3SessionStorage {
public:

    bool open(const std::string &db_path) ;

    ~SQLite3SessionStorage() {
        if ( handle_ ) sqlite3_close(handle_);
    }

    void execute(const std::string &cmd) ;
    void writeSessionData(const std::string &id, const string &data);
    void readSessionData(const std::string &id, string &data);
    void deleteSessions(uint64_t t);
    bool contains(const string &id);

private:
    sqlite3 *handle_ = nullptr ;

};

class SQLite3SessionStorageException: public std::runtime_error {
public:
    SQLite3SessionStorageException(const string &msg, sqlite3 *handle):
        std::runtime_error(msg + ": " + sqlite3_errmsg(handle)) {}

    SQLite3SessionStorageException(const string &msg, const char *error): std::runtime_error(msg + ": " + error) {}
};

bool SQLite3SessionStorage::open(const std::string &db_path) {

    if ( sqlite3_open_v2(db_path.c_str(), &handle_, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX, nullptr) )
        throw SQLite3SessionStorageException("Can't open session storage database", handle_) ;

    try {
        execute("PRAGMA auto_vacuum = 1") ;
        execute("PRAGMA journal_mode = WAL");
        execute("PRAGMA synchronous = NORMAL") ;
        execute("CREATE TABLE IF NOT EXISTS sessions ( sid TEXT PRIMARY KEY NOT NULL, data BLOB DEFAULT NULL, ts INTEGER NOT NULL );") ;
        execute("CREATE UNIQUE INDEX IF NOT EXISTS sessions_index ON sessions (sid);") ;
        return true ;
    }
    catch ( SQLite3SessionStorageException & ) {
        return false ;
    }
}

void SQLite3SessionStorage::execute(const string &cmd)
{
    char *error_msg ;
    if ( sqlite3_exec(handle_, cmd.c_str(), nullptr, nullptr, &error_msg) ) {
        throw SQLite3SessionStorageException("Error executing sql command", error_msg) ;
    }
}

void SQLite3SessionStorage::writeSessionData(const std::string &id, const string &data) {

    sqlite3_stmt *stmt = nullptr ;

    if ( sqlite3_prepare_v2(handle_, "REPLACE INTO sessions (sid, data, ts) VALUES (?, ?, ?)", -1, &stmt, nullptr) != SQLITE_OK ) {
        throw SQLite3SessionStorageException("Error executing sql command", handle_) ;
    }

    if ( sqlite3_bind_text(stmt, 1, id.c_str(), id.length(), nullptr) != SQLITE_OK  ||
         sqlite3_bind_blob(stmt, 2, data.data(), data.size(), nullptr) != SQLITE_OK ||
         sqlite3_bind_int64(stmt, 3, std::chrono::system_clock::now().time_since_epoch().count()) != SQLITE_OK ) {
        throw SQLite3SessionStorageException("Error executing sql prepared statement bind command", handle_) ;
    }

    if ( sqlite3_step(stmt) != SQLITE_DONE )
        throw SQLite3SessionStorageException("Error writing session data", handle_) ;


    sqlite3_finalize(stmt);
}

void SQLite3SessionStorage::readSessionData(const std::string &id, string &data) {

    sqlite3_stmt *stmt = nullptr ;

    if ( sqlite3_prepare_v2(handle_, "SELECT data FROM sessions WHERE sid = ? LIMIT 1", -1, &stmt, nullptr) != SQLITE_OK ) {
        throw SQLite3SessionStorageException("Error executing sql command", handle_) ;
    }

    if ( sqlite3_bind_text(stmt, 1, id.c_str(), id.length(), nullptr) != SQLITE_OK  ) {
        throw SQLite3SessionStorageException("Error executing sql prepared statement bind command", handle_) ;
    }

    if ( sqlite3_step(stmt) != SQLITE_ROW )
        throw SQLite3SessionStorageException("Error reading session data", handle_) ;

    const char *blob = (const char *)sqlite3_column_blob(stmt, 0) ;
    int blob_sz = sqlite3_column_bytes(stmt, 0) ;

    data.assign(blob, blob_sz) ;

    sqlite3_finalize(stmt);

}

void SQLite3SessionStorage::deleteSessions(uint64_t t) {
    sqlite3_stmt *stmt = nullptr ;

    if ( sqlite3_prepare_v2(handle_, "DELETE FROM sessions WHERE ts < ?", -1, &stmt, nullptr) != SQLITE_OK ) {
        throw SQLite3SessionStorageException("Error executing sql command", handle_) ;
    }

    if ( sqlite3_bind_int64(stmt, 1, t) != SQLITE_OK ) {
        throw SQLite3SessionStorageException("Error executing sql prepared statement bind command", handle_) ;
    }

    if ( sqlite3_step(stmt) != SQLITE_DONE )
        throw SQLite3SessionStorageException("Error deleting session data", handle_) ;


    sqlite3_finalize(stmt);
}

bool SQLite3SessionStorage::contains(const string &id) {
    sqlite3_stmt *stmt = nullptr ;

    if ( sqlite3_prepare_v2(handle_, "SELECT sid FROM sessions WHERE sid = ? LIMIT 1", -1, &stmt, nullptr) != SQLITE_OK ) {
        throw SQLite3SessionStorageException("Error executing sql command", handle_) ;
    }

    if ( sqlite3_bind_text(stmt, 1, id.c_str(), id.length(), nullptr) != SQLITE_OK ) {
        throw SQLite3SessionStorageException("Error executing sql prepared statement bind command", handle_) ;
    }

    int rc = sqlite3_step(stmt) ;

    sqlite3_finalize(stmt);

    if ( rc == SQLITE_DONE )
        return false ;
    else if ( rc == SQLITE_ROW )
        return true ;
    else
       throw SQLite3SessionStorageException("Error quering session data", handle_) ;
}
/////////////////////////////////////////////////////////////////////////////////////////////

FileSystemSessionHandler::FileSystemSessionHandler(const std::string &db_path)  {
    storage_.reset(new SQLite3SessionStorage()) ;
    storage_->open(db_path) ;
}

bool FileSystemSessionHandler::open() {
    return true ;
}

bool FileSystemSessionHandler::close() {
    return true ;
}

string FileSystemSessionHandler::uniqueSID() {
    int max_tries = 4 ;
    while ( max_tries > 0 ) {
        string sid = generateSID() ;
        if ( !sid.empty() && !contains(sid) ) return sid ;
        else max_tries -- ;
    }
    return string() ;
}

enum
{
    O32_LITTLE_ENDIAN = 0x03020100ul,
    O32_BIG_ENDIAN = 0x00010203ul,
    O32_PDP_ENDIAN = 0x01000302ul
};


static bool platform_is_little_endian() {
    static const union { unsigned char bytes[4]; uint32_t value; } o32_host_order =
    { { 0, 1, 2, 3 } };

    return ( o32_host_order.value == 0x03020100ul ) ;
}

static void byte_swap_32(uint32_t &data)
{
    union u {uint32_t v; uint8_t c[4];};
    u un, vn;
    un.v = data ;
    vn.c[0]=un.c[3];
    vn.c[1]=un.c[2];
    vn.c[2]=un.c[1];
    vn.c[3]=un.c[0];
    data = vn.v ;
}

static uint32_t read_uint32(istream &strm) {
    uint32_t i ;
    if ( platform_is_little_endian() )
        strm.read((char *)&i, 4) ;
    else {
        strm.read((char *)&i, 4) ;
        byte_swap_32(i) ;
    }
    return i ;
}

static void write_uint32(ostream &strm, uint32_t i) {
    if ( platform_is_little_endian() )
        strm.write((char *)&i, 4) ;
    else {
        byte_swap_32(i) ;
        strm.write((char *)&i, 4) ;
    }
}

static void write_string(ostream &strm, const string &str) {
    write_uint32(strm, str.length()) ;
    strm.write(str.data(), str.length()) ;
}

static string read_string(istream &strm) {
    uint32_t len = read_uint32(strm) ;
    string res ;
    res.resize(len) ;
    strm.read((char *)res.data(), len) ;
    return res ;
}

FileSystemSessionHandler::~FileSystemSessionHandler() = default ;

string FileSystemSessionHandler::serializeData(const Dictionary &data)
{
    ostringstream strm(ios::out | ios::binary) ;

    write_uint32(strm, data.size()) ;

    for( auto &p: data ) {
        write_string(strm, p.first) ;
        write_string(strm, p.second) ;
    }

    return strm.str() ;
}

void FileSystemSessionHandler::deserializeData(const string &data, Dictionary &dict)
{
    istringstream strm(data, ios::in | ios::binary) ;

    uint32_t len = read_uint32(strm) ;

    for( uint i=0 ; i<len ; i++ ) {
        string key = read_string(strm) ;
        string val = read_string(strm) ;
        dict[key] = val ;
    }
}

bool FileSystemSessionHandler::writeSessionData(const string &id, const string &data)
{
    try {

        storage_->writeSessionData(id, data) ;

        return true ;
    }
    catch ( SQLite3SessionStorageException & ) {
       return false ;
    }

}

bool FileSystemSessionHandler::readSessionData(const string &id, string &data)
{
    try {
        storage_->readSessionData(id, data) ;
       return true ;
    }
    catch ( SQLite3SessionStorageException &e ) {
       cerr << e.what() << endl ;
       return false ;
    }
}

bool FileSystemSessionHandler::contains(const string &id) {
    return storage_->contains(id) ;

}

bool FileSystemSessionHandler::write(const Session &session) {
    string id = session.id() ;
    string data = serializeData(session.data()) ;
    return writeSessionData(id, data) ;
}


bool FileSystemSessionHandler::read(Session &session) {
    string id = session.id(), data ;
    if ( !readSessionData(id, data) ) return false ;
    deserializeData(data, session.data()) ;
    gc() ;
    return true ;
}

// php like session garbage collection

void FileSystemSessionHandler::gc() {
    static const auto session_entry_max_lifetime = std::chrono::minutes(60) ;
    static const uint64_t session_gc_probability = 1 ;
    static const uint64_t session_gc_divisor = 1000 ;
    static std::random_device session_gc_rnd_device ;
    static std::default_random_engine session_gc_rnd_engine(session_gc_rnd_device());

    std::uniform_int_distribution<uint64_t> uniform_dist(1, session_gc_divisor);
    uint64_t p =  uniform_dist(session_gc_rnd_engine) ;

    if ( p  <= session_gc_probability ) {

        // delete old records

        auto t = std::chrono::system_clock::now() - session_entry_max_lifetime ;

        storage_->deleteSessions((uint64_t)t.time_since_epoch().count()) ;
    }

}


} // namespace ws
