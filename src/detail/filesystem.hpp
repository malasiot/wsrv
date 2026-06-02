#ifndef WS_FILESYSTEM_HPP
#define WS_FILESYSTEM_HPP

#include <string>
#include <filesystem>

namespace ws {

// read whole file to memory
std::string readFileToString(const std::string &fileName) ;

bool fileExists(const std::string &p) ;

std::filesystem::file_time_type fileLastWriteTime(const std::string &p) ;
}


#endif
