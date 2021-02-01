//
// request_parser.hpp
// ~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef WS_REQUEST_PARSER_HPP
#define WS_REQUEST_PARSER_HPP

#include <map>

#include "http_parser.h"

namespace ws {

struct Request;

namespace detail {

enum { HTTP_PARSER_OK = 0, HTTP_PARSER_ERROR = 1, HTTP_PARSER_INDETERMINATE = 2 } ;
/// Parser for incoming requests.
class RequestParser
{
public:
    /// Construct ready to parse the request method.
    RequestParser();

    /// Reset to initial parser state.
    void reset();

    /// call the parser with chunk of data while it is in indeterminate state
    int parse(const char *data, size_t buf_len) ;

    // fill in the request structure
    bool decode_message(Request &req) const ;

private:

    static int on_message_begin(http_parser * parser);
    static int on_message_complete(http_parser *parser);
    static int on_header_field(http_parser *parser, const char *data, size_t size);
    static int on_header_value(http_parser *parser, const char *data, size_t size);
    static int on_headers_complete (http_parser * parser);
    static int on_url(http_parser * parser, const char *data, size_t size);
    static int on_body(http_parser * parser, const char *data, size_t size) ;

    static bool parse_form_data(Request &session, std::istream &strm);
    static bool parse_url(Request &req, const std::string &url);
    static bool parse_cookie(Request &session, const std::string &data) ;
    static bool parse_cookies(Request &session) ;
    static bool parse_mime_data(Request &session, std::istream &strm, const std::string &fld, const std::string &file_name,
                              const std::string &content_type, const std::string trans_encoding, const std::string &bnd);
    static bool parse_multipart_data(Request &session, std::istream &strm, const std::string &bnd) ;


protected:
    http_parser parser_ ;
    http_parser_settings settings_ ;

    std::string current_header_field_, current_header_value_, url_, body_, protocol_ ;
    std::map<std::string, std::string> headers_ ;
    bool is_complete_ ;
};

} // namespace detail
} // namespace ws

#endif
