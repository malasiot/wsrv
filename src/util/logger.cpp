#include <wsrv/util/logger.hpp>
#include <filesystem>

#include <iomanip>
#include <iostream>
#include <vector>
#include <sstream>

#include <zlib.h>

namespace fs = std::filesystem;
using namespace std ;

namespace ws {

void FileLogger::open() {
    if (file_stream_.is_open()) {
            file_stream_.close();
    }
    file_stream_.open(log_path_, std::ios::out | std::ios::app);
}

static std::string get_timestamp() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
        
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

static bool compress_file_gzip(const fs::path& source, const fs::path& destination) {
    std::ifstream in(source, std::ios::binary);
    if (!in.is_open()) return false;

    gzFile out = gzopen(destination.string().c_str(), "wb9");
    if (!out) return false;

    std::vector<char> buffer(16384); // 16 KB chunk buffer
    while (in.read(buffer.data(), buffer.size()) || in.gcount() > 0) {
        int bytes_written = gzwrite(out, buffer.data(), static_cast<unsigned>(in.gcount()));
        if (bytes_written <= 0) {
            gzclose(out);
            return false;
        }
    }

    gzclose(out);
    return true;
}

static std::string to_string(LogLevel level) {
    switch (level) {
        case LogLevel::Debug:    return "DEBUG";
        case LogLevel::Info:     return "INFO";
        case LogLevel::Warning:  return "WARN";
        case LogLevel::Error:    return "ERROR";
        case LogLevel::Critical: return "CRIT";
        default:                 return "UNKNOWN";
    }
}

void FileLogger::log(LogLevel lv, const std::string &msg) {
    std::lock_guard<std::mutex> lock(mutex_);
        
    // Check file size and rotate if necessary
    if (fs::exists(log_path_) && fs::file_size(log_path_) >= max_file_sz_) {
        rotateLogs();
    }

    file_stream_ << get_timestamp() << " " << to_string(lv) << " " << msg << "\n";
    file_stream_.flush();
}

void FileLogger::rotateLogs() {
    file_stream_.close();

    // Delete the oldest compressed file if it exceeds max_files
    fs::path oldest_gz( log_path_ + "." + std::to_string(max_log_file_idx_) + ".gz" );
    if (fs::exists(oldest_gz)) {
        fs::remove(oldest_gz);
    }

    // Shift existing compressed files down the chain (.1.gz -> .2.gz)
    for (size_t i = max_log_file_idx_; i > 0; --i) {
        fs::path old_name = log_path_ + "." + std::to_string(i) + ".gz";
        fs::path new_name = log_path_ + "." + std::to_string(i + 1) + ".gz";
        if (fs::exists(old_name)) {
            fs::rename(old_name, new_name);
        }
    }

    string target_gz = log_path_ + ".1.gz";
        
    // Compress the current active log directly into the target .gz file
    if ( compress_file_gzip(log_path_, target_gz) ) {
        fs::remove(log_path_); // Delete the uncompressed raw file only if zlib succeeded
    } else {
        std::cerr << "zlib compression failed for: " << log_path_ << "\n";
        // Fallback strategy: rename it to .1 uncompressed so data isn't overwritten
        fs::rename(log_path_, log_path_ + ".1"); 
    }

    // Start a fresh log file
    open();
}


void DebugLogger::log(LogLevel l, const std::string &msg) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::cout << get_timestamp() << " " << to_string(l) << " " << msg << std::endl;
}

}