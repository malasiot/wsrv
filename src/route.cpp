#include <wsrv/route.hpp>

#include <cassert>
#include <mutex>
#include <iostream>
#include <vector>
#include <regex>

using namespace std ;

namespace ws {

struct RouteElement {
    RouteElement(const string &name, const string &pattern, bool optional, int idx = -1):
        name_(name), pattern_(pattern), optional_(optional), idx_(idx) {}

    string name_ ;
    string pattern_ ;
    int idx_ = -1;
    bool optional_ ;
};


class UriPatternMatcher {
    using Dictionary = map<string, string>;
public:
    UriPatternMatcher(RouteImpl &route) ;
    bool matches(const string &uri, Dictionary &vars) ;

private:
    std::regex makeRegexFromPattern(const string &pat, RouteImpl &route_data);
    bool matchPattern(const std::regex &rx, const RouteImpl &route, const string &path, Dictionary &vars) ;

    std::regex rx_ ;
    RouteImpl &route_ ;
};


struct RouteImpl {
    using Dictionary = map<string, string>;
public:
    RouteImpl(const std::string &pattern) {
        if ( pattern.back() == '/' ) pattern_ = pattern ;
        else pattern_ = pattern + '/' ;
        assert(parse(pattern_));
        makeRegexFromPattern() ;
    }

    bool parse(const string &pattern) ;
    void makeRegexFromPattern();

    bool match(const string &path, Dictionary &vars);
    string url(const Dictionary &params, bool relative) const;

    vector<RouteElement> elements_ ;
    string pattern_ ;
    std::regex rx_ ;
};


bool RouteImpl::parse(const string &pattern) {
    string::const_iterator cursor = pattern.begin(), end = pattern.end() ;

    enum State { Token, Element, Param, Regex } ;

    State state = Element ;

    string name, pat ;
    bool is_optional = false ;
    uint idx = 0 ;

    if ( cursor != end && *cursor == '/' ) ++cursor ; // omit leading /

    while ( cursor != end ) {
        char c = *cursor++ ;
        if ( state == Token ) {
            if ( c == '/' || cursor == end ) {
                if ( name.empty() )
                    elements_.emplace_back(RouteElement(name, pat, is_optional )) ;
                else
                    elements_.emplace_back(RouteElement(name, pat, is_optional, ++idx )) ;

                name.clear() ; pat.clear() ; is_optional = false ;
                state = Element ;
            }
            else if ( c == '?') { is_optional = true ; }
            else return false ;
        }
        else if ( state == Param ) {
            if ( c == ':' )
                state = Regex ;
            else if ( c == '}' )
                state = Token ;
            else if ( isalnum(c) || c == '_' || c == '-' ) name.push_back(c) ;
            else return false ;
        }
        else if ( state == Regex ) {
            if ( c == '}' ) state = Token ;
            else pat.push_back(c) ;
        }
        else if ( state == Element ) {
            if ( c == '{' ) {
                if ( !pat.empty() ) return false ;
                state = Param ;
            }
            else if ( cursor == end )
                elements_.emplace_back(RouteElement(name, pat, is_optional)) ;
            else if ( c == '/' ) {
                state = Token ;
                cursor -- ;
            }
            else pat.push_back(c) ;
        }
    }


    return true ;
}

bool RouteImpl::match(const string &path, Dictionary &vars)
{
    // TODO implement without named capture groups

    std::smatch results ;
    if ( std::regex_match(path, results, rx_) ) {
        for( const RouteElement &e: elements_ ) {
            if ( e.idx_ > 0 ) {
                string val = results[e.idx_].str() ;
                if ( !val.empty() ) vars[e.name_] = val ;
            }
        }
        return true ;
    }

    return false ;
}

void RouteImpl::makeRegexFromPattern() {
    string rx ;

    vector<string> patterns ;
    uint capture_idx = 1 ;
    for( RouteElement &e: elements_ ) {
        string param = e.name_ ;
        string pattern = e.pattern_ ;

        if ( pattern.empty() ) pattern = "[^\\/]+" ;

        if ( !param.empty() ) {
            patterns.push_back("(" + pattern +  ")" ) ;
            e.idx_ = capture_idx ++ ;
        }
        else
            patterns.push_back( "(?:" + pattern + ")") ;
    }

    auto it = elements_.begin() ;
    auto pit = patterns.begin() ;

    for(  ; it != elements_.end() ; ++it, ++pit ) {
        const RouteElement &e = *it ;

        if ( e.optional_ ) rx += "(?:" + *pit + "\\/)?"  ;
        else rx += "(?:" + *pit + "\\/)";
    }

    rx = "^\\/" + rx + '$';


    try {
        rx_.assign(rx) ;
    }
    catch ( std::regex_error &e ) {
        std::cout << e.what() << endl ;
    }
}

string RouteImpl::url(const Dictionary &params, bool relative) const {
    string res ;
    if ( !relative ) res += '/' ;
    for( const RouteElement &e: elements_ ) {
        if ( e.name_.empty() ) {
            res += e.pattern_ + '/' ;
        }
        else {
            auto it = params.find(e.name_) ;
            if ( it == params.end() ) {
                if ( e.optional_ ) return res ;
                else assert(1) ;
            }
            else {
                res += it->second + '/' ;
            }
        }
    }

    return res ;
}


Route::Route(const string &pattern): impl_(new RouteImpl(pattern)) {
}

Route::~Route() {}

static string get_clean_path(const std::string &path)
{
    if ( path.back() != '/' ) return path + '/' ;
    else return path ;
}

bool Route::matches(const string &path, Dictionary &data) const {
    return impl_->match(get_clean_path(path), data) ;

}

bool Route::matches(const string &path) const {
    Dictionary data ;
    return matches(path, data) ;
}

string Route::url(const Dictionary &params, bool relative) const  {
    return impl_->url(params, relative) ;
}

} // namespace ws

