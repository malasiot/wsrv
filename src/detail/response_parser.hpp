#ifndef WS_RESPONSE_PARSER_HPP
#define WS_RESPONSE_PARSER_HPP

#include <map>
#include <string>

#include "http_parser.h"

namespace ws {

struct HTTPServerResponse;

namespace detail {

enum { HTTP_PARSER_OK = 0, HTTP_PARSER_ERROR = 1, HTTP_PARSER_INDETERMINATE = 2 } ;
/// Parser for incoming requests.
class ResponseParser
{
public:
    ResponseParser();

    void reset();

    /// call the parser with chunk of data while it is in indeterminate state
    int parse(const char *data, size_t buf_len) ;

    // fill in the request structure
    bool decode_message(HTTPServerResponse &resp) const ;

private:

    static int on_message_begin(http_parser * parser);
    static int on_message_complete(http_parser *parser);
    static int on_header_field(http_parser *parser, const char *data, size_t size);
    static int on_status(http_parser *parser, const char *data, size_t size);
    static int on_header_value(http_parser *parser, const char *data, size_t size);
    static int on_headers_complete (http_parser * parser);
    static int on_url(http_parser * parser, const char *data, size_t size);
    static int on_body(http_parser * parser, const char *data, size_t size) ;

protected:
    http_parser parser_ ;
    http_parser_settings settings_ ;

    std::string current_header_field_, current_header_value_, url_, body_, protocol_, status_message_ ;
    std::map<std::string, std::string> headers_ ;
    bool is_complete_ ;
};

} // namespace detail
} // namespace ws

#endif
