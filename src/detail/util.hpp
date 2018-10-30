#ifndef WS_UTIL_HPP
#define WS_UTIL_HPP

#include <vector>
#include <string>
#include <regex>

namespace ws {

std::vector<std::string> split(const std::string &s, const char *delimeters) ;

std::vector<std::string> split(const std::string &s, const std::regex &re);

std::string join(const std::vector<std::string> &tokens, const char *del) ;

std::string rtrimCopy(const std::string &str, const char *delim = " \t\n\r") ;

void rtrim(std::string &str, const char *delim = " \t\n\r") ;

void ltrim(std::string &str, const char *delim = " \t\n\r") ;

std::string ltrimCopy(const std::string &str, const char *delim = " \t\n\r") ;

std::string trimCopy(const std::string &str, const char *delim = " \t\n\r") ;

void trim(std::string &str, const char *delim = " \t\n\r") ;

bool startsWith(const std::string &src, const std::string &prefix) ;

bool endsWith(const std::string &src, const std::string &suffix) ;
}


#endif
