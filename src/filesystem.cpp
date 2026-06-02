#include "detail/filesystem.hpp"

#include <fstream>
#include <filesystem>
#include <sys/types.h>
#include <sys/stat.h>

using namespace std ;
namespace fs = std::filesystem ;

namespace ws {

string readFileToString(const string &fileName)
{
    ifstream strm(fileName) ;

    if ( !strm ) return string() ;

    strm.seekg(0, ios::end);
    size_t length = strm.tellg();
    strm.seekg(0,std::ios::beg);

    string res ;
    res.resize(length) ;
    strm.read(&res[0], length) ;
    return res ;
}

bool fileExists(const string &p) {
    return fs::exists(p) && fs::is_regular_file(p);
}

fs::file_time_type fileLastWriteTime(const std::string &p) {
    return fs::last_write_time(p);
}


}
