#ifndef WS_FILESYSTEM_HPP
#define WS_FILESYSTEM_HPP

#include <string>

namespace ws {

// read whole file to memory
std::string readFileToString(const std::string &fileName) ;

bool fileExists(const std::string &p) ;

time_t fileLastWriteTime(const std::string &p) ;
}


#endif
