#ifndef WS_REQUEST_HPP
#define WS_REQUEST_HPP

#include <string>
#include <vector>

#include <wsrv/route.hpp>
#include <wsrv/middleware_data.hpp>

namespace ws {

class Session ;
class HttpConnection ;

namespace detail {
class RequestParser ;
}
/// A request received from a client.
///
class SessionManager ;
class HTTPServerRequest
{
    using Dictionary = std::map<std::string, std::string> ;

public:
    // helper function to match a request with a Route pattern (see Route)
    bool matches(const std::string &method, const Route &pattern, Dictionary &attributes) const ;
    bool matches(const std::string &method, const Route &pattern) const;
    bool matches(const std::string &method, const std::string &pattern, Dictionary &attributes) const ;
    bool matches(const std::string &method, const std::string &pattern) const;

    bool supportsGzip() const ;

    struct UploadedFile {
        std::string name_ ;	// The original filename
        std::string path_ ; // The path of a local temporary copy of the uploaded file
        std::string mime_ ;	// MIME information of the uploaded file
        size_t size_ ;
        std::string data_ ; // This member variable contains the file contents for small files
    } ;

    const Dictionary &getServerAttributes() const { return SERVER_ ; }
    std::string getServerAttribute(const std::string &key, const std::string &def = std::string()) const ;

    const Dictionary &getQueryAttributes() const { return GET_ ; }
    std::string getQueryAttribute(const std::string &key, const std::string &def = std::string()) const ;

    const Dictionary &getPostAttributes() const { return POST_ ; }
    std::string getPostAttribute(const std::string &key, const std::string &def = std::string()) const ;

    Dictionary &getRouteAttributes() { return ROUTE_ ; }
    std::string getRouteAttribute(const std::string &key, const std::string &def = std::string()) const ;

    MiddlwareDataStorage &data() { return mw_data_ ; }

    const std::map<std::string, UploadedFile> &getUploadedFiles() const { return FILE_ ; }
    const Dictionary &getCookies() const { return COOKIE_ ; }
    std::string getCookie(const std::string &key, const std::string &def = std::string()) const ;

    const std::string &getContent() const { return content_ ; }
    const std::string &getContentType() const { return content_type_ ; }
    const std::string &getMethod() const { return method_ ; }
    const std::string &getPath() const { return path_ ; }
    const std::string &getQueryString() const { return query_ ; }
    const std::string &getProtocol() const { return protocol_ ; }

    // converts to single line string suitable for logging the request
    std::string toString() const ;

    bool matchesMethod(const std::string &method) const ;


protected:

    friend class detail::RequestParser ;

    Dictionary SERVER_ ; // Server variables
    Dictionary GET_ ;	 // Query variables for GET requests
    Dictionary POST_ ;   // Post variables for POST requests
    Dictionary COOKIE_ ; // Cookies
    Dictionary ROUTE_ ;  // Attributes extracted from the request path (e.g. id in /user/{id})
   
    std::map<std::string, UploadedFile> FILE_ ;	// Uploaded files

    MiddlwareDataStorage mw_data_ ;

    // This is content sent using POST with Content-Type other than
    // x-www-form-urlencoded or multipart-form-data e.g. text/xml

    std::string content_ ;
    std::string content_type_ ;

    std::string method_;  // request method GET/POST etc
    std::string path_;    // decoded request path
    std::string query_ ;  // decoded query string
    std::string protocol_ ; // HTTP protocol
    
private:

    friend class HttpConnection ;

    bool session_is_valid_ = false ;
    std::string session_id_ ;

    HTTPServerRequest() = default ;

    std::string getCleanPath(const std::string &path) const ;
};



} // namespace ws

#endif
