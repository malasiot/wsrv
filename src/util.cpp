#include "detail/util.hpp"

#include <cstring>
#include <regex>

using namespace std ;

namespace ws {


std::vector<string> split(const string &s, const char *delimeters)
{
    vector<string> tokens;
    string token;

    for( char c: s ) {
        if ( strchr(delimeters, c) == nullptr )
            token += c ;
        else  {
            if ( !token.empty() )
                tokens.push_back(token);
            token.clear();
        }
    }
    if ( !token.empty() ) tokens.push_back(token);

    return tokens ;
}

std::vector<string> split(const string &s, const regex &re)
{
    sregex_token_iterator first{s.begin(), s.end(), re, -1}, last;
    return {first, last};
}

string rtrimCopy(const string &str, const char *delim) {
    size_t endpos = str.find_last_not_of(delim);
    if( string::npos != endpos )
        return str.substr( 0, endpos+1 );
    else
        return str ;
}

void rtrim(string &str, const char *delim) {
    size_t endpos = str.find_last_not_of(delim);
    if( string::npos != endpos )
       str.substr( 0, endpos+1 ).swap(str);
}

void ltrim(string &str, const char *delim) {
    size_t startpos = str.find_first_not_of(delim);
    if( string::npos != startpos )
        str.substr( startpos ).swap(str);
}

string ltrimCopy(const string &str, const char *delim) {
    size_t startpos = str.find_first_not_of(delim);
    if( string::npos != startpos )
        return str.substr( startpos ) ;
    else
        return str ;
}

string trimCopy(const string &str, const char *delim) {
    return ltrimCopy(rtrimCopy(str, delim), delim) ;
}

void trim(string &str, const char *delim) {
    ltrim(str, delim) ;
    rtrim(str, delim) ;
}


bool startsWith(const string &src, const string &prefix)
{
    if ( src.length() >= prefix.length() )
        return src.compare(0, prefix.size(), prefix) == 0;
    else
        return false ;
}

bool endsWith(const string &src, const string &suffix)
{
    if ( src.length() >= suffix.length() )
        return src.compare ( src.length() - suffix.length(), suffix.length(), suffix) == 0;
    else
        return false ;
}


void replaceAll(string &subject, const string &search, const string &replace)
{
    size_t pos = 0;
    while ( (pos = subject.find(search, pos)) != std::string::npos) {
        subject.replace(pos, search.length(), replace);
        pos += replace.length();
    }
}

string replaceAllCopy(const string &subject, const string &search, const string &replace)
{
    string res ;
    size_t pos = 0, start_pos = 0;
    while ( (pos = subject.find(search, pos)) != std::string::npos) {
        res.append(subject.substr(start_pos, pos - start_pos)) ;
        res.append(replace) ;
        pos += search.length();
        start_pos = pos;
    }
    return res ;
}

string toUpperCopy(const std::string &source) {
    string res ;
    std::transform(source.begin(), source.end(), std::back_inserter(res), ::toupper);
    return res ;
}

void toUpper(string &source) {
    std::transform(source.begin(), source.end(), source.begin(), ::toupper);
}

string toLowerCopy(const std::string &source) {
    string res ;
    std::transform(source.begin(), source.end(), std::back_inserter(res), ::tolower);
    return res ;
}

void toLower(string &source) {
    std::transform(source.begin(), source.end(), source.begin(), ::tolower);
}


string replace(string &subject, const regex &search, const string &replace)
{
    return regex_replace(subject, search, replace) ;
}

string replace(string &src, const regex &re, std::function<string (const smatch &)> callback)
{
    string res ;
    size_t pos = 0, start_pos = 0;

    sregex_iterator  begin(src.begin(), src.end(), re),  end;
    std::for_each(begin, end, [&](const smatch &match) {
        pos = match.position((size_t)0);
        res.append(src.substr(start_pos, pos - start_pos)) ;
        res.append(callback(match)) ;
        pos += match.length(0) ;
        start_pos = pos ;
    });

    return res;
}

string join(const std::vector<string> &parts, const char *delimeter)
{
    string s ;
    for( auto it = parts.begin() ; it != parts.end() ; ++it ) {
        if ( it != parts.begin() ) s += delimeter ;
        s += *it ;
    }
    return s ;
}

static void hexchar(unsigned char c, unsigned char &hex1, unsigned char &hex2)
{
    hex1 = c / 16;
    hex2 = c % 16;
    hex1 += hex1 <= 9 ? '0' : 'a' - 10;
    hex2 += hex2 <= 9 ? '0' : 'a' - 10;
}

std::string url_encode(const std::string &s) {
    const char *str = s.c_str();
    vector<char> v(s.size());
    v.clear();
    for (size_t i = 0, l = s.size(); i < l; i++)
    {
        char c = str[i];
        if ((c >= '0' && c <= '9') ||
            (c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            c == '-' || c == '_' || c == '.' || c == '!' || c == '~' ||
            c == '*' || c == '\'' || c == '(' || c == ')')
        {
            v.push_back(c);
        }
        else if (c == ' ')
        {
            v.push_back('+');
        }
        else
        {
            v.push_back('%');
            unsigned char d1, d2;
            hexchar(c, d1, d2);
            v.push_back(d1);
            v.push_back(d2);
        }
    }

    return string(v.cbegin(), v.cend());
}


static int hex_decode(char c)
{
    char ch = tolower(c) ;

    if ( ch >= 'a' && ch <= 'f' ) return 10 + ch - 'a' ;
    else if ( ch >= '0' && ch <= '9' ) return ch - '0' ;
    else return 0 ;
}

string url_decode(const string &src) {
    const char *p = src.c_str() ;

    std::string ret ;
    while ( *p )
    {
        if( *p == '+' ) ret += ' ' ;
        else if ( *p == '%' )
        {
            ++p ;
            char tmp[4];
            unsigned char val = 16 * ( hex_decode(*p++) )  ;
            val += hex_decode(*p) ;
            sprintf(tmp,"%c", val);
            ret += tmp ;
        } else ret += *p ;
        ++p ;
    }

    return ret;
}


}
