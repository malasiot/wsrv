#include <ws/request.hpp>
#include <ws/route.hpp>

using namespace std ;

namespace ws {

bool Request::matches(const string &method, const string &pattern, Dictionary &attributes) const
{
    return matchesMethod(method) && Route(pattern).matches(path_, attributes) ;
}

bool Request::matches(const string &method, const string &pattern) const
{
    Dictionary attributes ;
    return matchesMethod(method) && Route(pattern).matches(path_, attributes) ;
}

bool Request::matches(const string &method, const Route &pattern, Dictionary &attributes) const
{
    return matchesMethod(method) && pattern.matches(path_, attributes) ;
}

bool Request::matches(const string &method, const Route &pattern) const
{
    Dictionary attributes ;
    return matchesMethod(method) && pattern.matches(path_, attributes) ;
}

bool Request::matchesMethod(const string &method) const {
    vector<string> methods ;

    static std::regex regex{R"([\s|]+)"}; // split on space and caret
    std::sregex_token_iterator it{method.begin(), method.end(), regex, -1};
    methods.assign(it, {}) ;

    return std::find(methods.begin(), methods.end(), method_) != methods.end() ;
}

} // namespace ws
