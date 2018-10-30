#ifndef __SERVER_REQUEST_HANDLER_HPP__
#define __SERVER_REQUEST_HANDLER_HPP__

#include <string>
#include <boost/noncopyable.hpp>

#include <ws/request.hpp>
#include <ws/response.hpp>

namespace ws {

/// The common handler for all incoming requests.
class RequestHandler: private boost::noncopyable
{
public:

    explicit RequestHandler() = default;

    /// Handle a request and produce a reply. Returns true if the request was handled (e.g. the request url and method match)
    /// or not.

    virtual void handle(const Request &req, Response &resp) = 0;
};


} // namespace ws

#endif
