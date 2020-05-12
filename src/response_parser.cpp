#include "detail/response_parser.hpp"
#include "detail/util.hpp"

#include <ws/response.hpp>

#include <algorithm>
#include <cstring>
#include <utility>
#include <fstream>

#include <algorithm>
#include <regex>

using namespace std ;

namespace ws {
namespace detail {

ResponseParser::ResponseParser()
{
    memset(&settings_, 0, sizeof(settings_));
    settings_.on_url = &on_url;

    settings_.on_message_complete = &on_message_complete;
    settings_.on_message_begin    = &on_message_begin;
    settings_.on_header_field     = &on_header_field;
    settings_.on_header_value     = &on_header_value;
    settings_.on_headers_complete = &on_headers_complete;
    settings_.on_status = &on_status ;
    settings_.on_body = &on_body;

    reset() ;
}

void ResponseParser::reset()
{
    memset(&parser_, 0, sizeof(parser_));
    http_parser_init(&parser_, HTTP_RESPONSE);

    parser_.data = this ;
    url_.clear() ;
    current_header_field_.clear() ;
    current_header_value_.clear() ;
    body_.clear() ;
}


int ResponseParser::on_message_begin(http_parser *parser) {
    ResponseParser &rp = *static_cast<ResponseParser*>(parser->data);
    rp.is_complete_ = false ;
    return 0;
}

int ResponseParser::on_message_complete(http_parser * parser)
{
    ResponseParser &rp = *static_cast<ResponseParser*>(parser->data);
    rp.is_complete_ = true ;

    // Force the parser to stop after the Request is parsed so clients
    // can process the Request (or response).  This is to properly
    // handle HTTP/1.1 pipelined Requests.
    http_parser_pause(parser, 1);

    return 0;
}

int ResponseParser::on_header_field(http_parser *parser, const char *data, size_t size)
{
    ResponseParser &rp = *static_cast<ResponseParser*>(parser->data);

    if ( !rp.current_header_value_.empty() ) {
        rp.headers_[rp.current_header_field_] = rp.current_header_value_ ;
        rp.current_header_field_.clear() ;
        rp.current_header_value_.clear() ;
    }
    rp.current_header_field_.append(data, size);
    return 0 ;
}

int ResponseParser::on_status(http_parser *parser, const char *data, size_t size)
{

     ResponseParser& rp = *static_cast<ResponseParser*>(parser->data);
     rp.status_message_.assign(data, size) ;
    return 0 ;
}

int ResponseParser::on_header_value(http_parser *parser, const char *data, size_t size)
{
    ResponseParser& rp = *static_cast<ResponseParser*>(parser->data);
    rp.current_header_value_.append(data, size);
    return 0 ;
}

int ResponseParser::on_headers_complete(http_parser * parser)
{
    ResponseParser &rp = *static_cast<ResponseParser*>(parser->data);

    if ( !rp.current_header_value_.empty() ) {
        rp.headers_[rp.current_header_field_] = rp.current_header_value_ ;
        rp.current_header_field_.clear() ;
        rp.current_header_value_.clear() ;
    }


    return 0;
}

int ResponseParser::on_url(http_parser *parser, const char *data, size_t size)
{
    ResponseParser& rp = *static_cast<ResponseParser*>(parser->data);
    rp.url_.assign(data, size);

    return 0;
}

int ResponseParser::on_body(http_parser * parser, const char *data, size_t size)
{
    static_cast<ResponseParser*>(parser->data)->body_.append(data, size) ;
    return 0;
}



int ResponseParser::parse(const char *data, size_t size)
{
    std::size_t used = http_parser_execute(&parser_, &settings_, data, size);

    const http_errno error = static_cast< http_errno >(parser_.http_errno);

  // The 'on_message_complete' and 'on_headers_complete' callbacks fail
  // on purpose to force the parser to stop between pipelined Requests.
  // This allows the clients to reliably detect the end of headers and
  // the end of the message.  Make sure the parser is always unpaused
  // for the next call to 'feed'.

  if (error == HPE_PAUSED) {
      http_parser_pause(&parser_, 0);
  }

  if (used < size)
  {
      if (error == HPE_PAUSED)
      {
          // Make sure the byte that triggered the pause
          // after the headers is properly processed.
          if ( !is_complete_ )
              used += http_parser_execute(&parser_, &settings_, data + used, size - used);
      }
      else {
          return HTTP_PARSER_ERROR ;
      }
  }

  return ( is_complete_ ? HTTP_PARSER_OK : HTTP_PARSER_INDETERMINATE ) ;
}


bool ResponseParser::decode_message(Response &resp) const {

    for( auto hdr: headers_ )
        resp.headers_.emplace(hdr.first, hdr.second) ;

    resp.content_ = body_ ;
    resp.status_ = parser_.status_code ;

    return true ;
}

} // namespace detail
} // namespace wspp
