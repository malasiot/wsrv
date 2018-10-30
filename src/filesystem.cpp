#include "detail/filesystem.hpp"

#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>

using namespace std ;

namespace ws {

string readFileToString(const string &fileName)
{
    ifstream strm(fileName) ;

    strm.seekg(0, ios::end);
    size_t length = strm.tellg();
    strm.seekg(0,std::ios::beg);

    string res ;
    res.resize(length) ;
    strm.read(&res[0], length) ;
    return res ;
}

bool fileExists(const string &p) {
    struct stat buffer;
    return ::stat(p.c_str(), &buffer) == 0 ;
}

time_t fileLastWriteTime(const std::string &p) {
    struct stat result;
    if ( ::stat(p.c_str(), &result) == 0 )
        return result.st_mtime;
    return 0 ;
}


}
