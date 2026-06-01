#ifndef WS_SQLITE3_SESSION_MANAGER_HPP
#define WS_SQLITE3_SESSION_MANAGER_HPP

#include <wsrv/session_manager.hpp>

#include <string>

namespace ws {

// File storage of session data based on an sqlite3 database

class SQLite3SessionStorage ;

class SQLite3SessionManager: public SessionManager {
public:

    using dictionary_t  = std::map<std::string, std::string> ;

    SQLite3SessionManager(const std::string &db_path) ;
    ~SQLite3SessionManager() ;
private:

    virtual bool open() override ;
    virtual bool close() override ;
    virtual bool write(const std::string &session_id, const dictionary_t &session) override ;
    virtual bool read(const std::string &id, dictionary_t &session_data) override ;
    std::string uniqueSID() override ;
    bool isValidId(const std::string &id) const override ;
    bool remove(const std::string &id) override ;

private:

    std::string serializeData(const dictionary_t &data) ;
    void deserializeData(const std::string &data, dictionary_t &cont) ;

    bool writeSessionData(const std::string &id, const std::string &data) ;
    bool readSessionData(const std::string &id, std::string &data) ;

    bool contains(const std::string &id) const ;

    void gc() ;

private:

    std::unique_ptr<SQLite3SessionStorage> storage_ ;
};


} // namespace ws

#endif
