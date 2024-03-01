#ifndef WS_RESPONSE_HPP
#define WS_RESPONSE_HPP

#include <string>
#include <vector>
#include <map>
#include <sstream>

namespace ws {

namespace detail {
class ResponseParser ;
}

struct HTTPServerResponse
{
    using Dictionary = std::map<std::string, std::string> ;
    /// The status of the reply.
    enum Status
    {
        ok = 200,
        created = 201,
        accepted = 202,
        no_content = 204,
        multiple_choices = 300,
        moved_permanently = 301,
        moved_temporarily = 302,
        not_modified = 304,
        bad_request = 400,
        unauthorized = 401,
        forbidden = 403,
        not_found = 404,
        internal_server_error = 500,
        not_implemented = 501,
        bad_gateway = 502,
        service_unavailable = 503
    } ;


    Dictionary &headers() { return headers_ ; }
    std::string &content() { return content_ ; }
    unsigned int getStatus() const { return status_ ; }

    std::string getHeaderAttribute(const std::string &key, const std::string &def = std::string()) const ;

    /// Get a stock reply.
    void stockReply(Status status);

    // This will correctly fill in the reply headers for sending over a file payload. It will also set status to OK.
    // If encoding is empty it will try to guess from the payload (gzip only supported)

    void encodeFileData(const std::string &bytes,
                     const std::string &encoding,
                     const std::string &mime,
                     const time_t mod_time
                     ) ;

    // This will load the file given its path name and call encode_file_data above.
    // If no mime is provided it will try to guess from the file extension

    void encodeFile(const std::string &path_name,
                     const std::string &encoding = std::string(),
                     const std::string &mime = std::string()
                     ) ;

    // Calls write() appropriately setting the content type
    void writeJSON(const std::string &json) ;

    // Will write a string and set content type and content length. It will also set status to OK.
    void write(const std::string &content, const std::string &mime = "text/html") ;

    // This should be used for incrementally outputting text. Content type and length have to provided afterwards
    void append(const std::string &content) ;

    // serve contents of file in filesystem under folder with given path. returns false if the file does not exist
    bool serveStaticFile(const std::string &folder, const std::string &path) ;

    // helper for appending arbitrary streamable types
    template <class T>
    void append(const T &t) {
        std::ostringstream strm ;
        strm << t ;
        append(strm.str()) ;
    }

    // setup header corresponding to cookie
    void setCookie(const std::string &name, const std::string &value,
                   time_t expires = 0,
                   const std::string &path = std::string(),
                   const std::string &domain = std::string(),
                   bool secure = false,
                   bool http_only = false
                   ) ;


    void addHeader(const std::string &key, const std::string &val) ;
    // set header for content-type
    void setContentType(const std::string &mime);
    // set header for content-length based on the current length of the content_ string
    void setContentLength() ;

    // set output status
    void setStatus(Status st) { status_ = st ; }

    // check if the content may benefit from comression by checking length and mime type
    bool contentBenefitsFromCompression() ;

    // gzip content before sending
    void compress();

    // convert to one line string (status + content length) suitable for logging
    std::string toString() const ;
private:

    friend class detail::ResponseParser ;
    /// The headers to be included in the reply.
    Dictionary headers_;

    /// The content to be sent in the reply.
    std::string content_;

    unsigned int status_ = not_found ;

};


} // namespace ws

#endif
