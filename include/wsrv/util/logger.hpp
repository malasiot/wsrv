#pragma once

#include <string>
#include <mutex>
#include <fstream>

namespace ws {

enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error,
    Critical
};

// Logger interface
class Logger {
public:
    virtual void log(LogLevel lv, const std::string &msg) = 0 ;
};

// File logger with rotation and compression
class FileLogger: public Logger {
public:
    FileLogger(const std::string &log_path, size_t max_file_sz = 1024 * 1024, size_t max_log_file_idx = 100):
        log_path_(log_path), max_file_sz_(max_file_sz), max_log_file_idx_(max_log_file_idx) {
            open() ;
        }

    void log(LogLevel lv, const std::string &msg) override;
    
private:

    void open() ;
    void rotateLogs() ;
  
    std::ofstream file_stream_;
    std::mutex mutex_;
    std::string log_path_ ; 
    size_t max_file_sz_ ;         // max size of file after which rotation happens
    size_t max_log_file_idx_ ;    // maximum number of rotated files to keep
    size_t last_log_file_idx_ ;
};

// writes to cout
class DebugLogger: public Logger {
public:

    DebugLogger() = default ;
    void log(LogLevel lv, const std::string &msg) override ;

private:

    std::mutex mutex_ ;
};


}