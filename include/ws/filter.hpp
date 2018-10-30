#ifndef WS_FILTER_HPP
#define WS_FILTER_HPP

#include <string>

#include <ws/request.hpp>
#include <ws/response.hpp>

namespace ws {

class FilterChain ;

/// Middleware handler
///
class Filter
{
public:

    Filter( const Filter& ) = delete;

    explicit Filter() = default;

    /// Handle a request and produce a reply. Should call chain.next before or after to act as pre-post filter.

    virtual void handle(Request &req, Response &resp, FilterChain &chain) = 0;
};

} // namespace ws

#endif
