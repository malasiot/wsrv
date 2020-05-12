
#include <ws/response.hpp>
#include <ws/zstream.hpp>

#include <ws/exceptions.hpp>
#include <asio/buffered_read_stream.hpp>

#include <regex>

#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <time.h>

#include "detail/filesystem.hpp"

using namespace std ;

namespace ws {

namespace status_strings {

const std::string ok =
        "HTTP/1.0 200 OK\r\n";
const std::string created =
        "HTTP/1.0 201 Created\r\n";
const std::string accepted =
        "HTTP/1.0 202 Accepted\r\n";
const std::string no_content =
        "HTTP/1.0 204 No Content\r\n";
const std::string multiple_choices =
        "HTTP/1.0 300 Multiple Choices\r\n";
const std::string moved_permanently =
        "HTTP/1.0 301 Moved Permanently\r\n";
const std::string moved_temporarily =
        "HTTP/1.0 302 Moved Temporarily\r\n";
const std::string not_modified =
        "HTTP/1.0 304 Not Modified\r\n";
const std::string bad_request =
        "HTTP/1.0 400 Bad Request\r\n";
const std::string unauthorized =
        "HTTP/1.0 401 Unauthorized\r\n";
const std::string forbidden =
        "HTTP/1.0 403 Forbidden\r\n";
const std::string not_found =
        "HTTP/1.0 404 Not Found\r\n";
const std::string internal_server_error =
        "HTTP/1.0 500 Internal Server Error\r\n";
const std::string not_implemented =
        "HTTP/1.0 501 Not Implemented\r\n";
const std::string bad_gateway =
        "HTTP/1.0 502 Bad Gateway\r\n";
const std::string service_unavailable =
        "HTTP/1.0 503 Service Unavailable\r\n";

asio::const_buffer to_buffer(unsigned int status)
{
    switch (status)
    {
    case Response::ok:
        return asio::buffer(ok);
    case Response::created:
        return asio::buffer(created);
    case Response::accepted:
        return asio::buffer(accepted);
    case Response::no_content:
        return asio::buffer(no_content);
    case Response::multiple_choices:
        return asio::buffer(multiple_choices);
    case Response::moved_permanently:
        return asio::buffer(moved_permanently);
    case Response::moved_temporarily:
        return asio::buffer(moved_temporarily);
    case Response::not_modified:
        return asio::buffer(not_modified);
    case Response::bad_request:
        return asio::buffer(bad_request);
    case Response::unauthorized:
        return asio::buffer(unauthorized);
    case Response::forbidden:
        return asio::buffer(forbidden);
    case Response::not_found:
        return asio::buffer(not_found);
    case Response::internal_server_error:
        return asio::buffer(internal_server_error);
    case Response::not_implemented:
        return asio::buffer(not_implemented);
    case Response::bad_gateway:
        return asio::buffer(bad_gateway);
    case Response::service_unavailable:
        return asio::buffer(service_unavailable);
    default:
        return asio::buffer(internal_server_error);
    }
}

} // namespace status_strings

namespace misc_strings {

const char name_value_separator[] = { ':', ' ' };
const char crlf[] = { '\r', '\n' };

} // namespace misc_strings


/// Convert the reply into a vector of buffers. The buffers do not own the
/// underlying memory blocks, therefore the reply object must remain valid and
/// not be changed until the write operation has completed.


std::vector<asio::const_buffer> response_to_buffers(Response &rep, bool is_head)
{
    std::vector<asio::const_buffer> buffers;
    buffers.push_back(status_strings::to_buffer(rep.getStatus()));

    for( const auto &h: rep.headers() )
    {
        buffers.push_back(asio::buffer(h.first));
        buffers.push_back(asio::buffer(misc_strings::name_value_separator));
        buffers.push_back(asio::buffer(h.second));
        buffers.push_back(asio::buffer(misc_strings::crlf));
    }
    if ( !is_head ) {
        buffers.push_back(asio::buffer(misc_strings::crlf));
        buffers.push_back(asio::buffer(rep.content()));
    }
    return buffers;
}

namespace stock_replies {

const char ok[] = "";
const char created[] =
        "<html>"
        "<head><title>Created</title></head>"
        "<body><h1>201 Created</h1></body>"
        "</html>";
const char accepted[] =
        "<html>"
        "<head><title>Accepted</title></head>"
        "<body><h1>202 Accepted</h1></body>"
        "</html>";
const char no_content[] =
        "<html>"
        "<head><title>No Content</title></head>"
        "<body><h1>204 Content</h1></body>"
        "</html>";
const char multiple_choices[] =
        "<html>"
        "<head><title>Multiple Choices</title></head>"
        "<body><h1>300 Multiple Choices</h1></body>"
        "</html>";
const char moved_permanently[] =
        "<html>"
        "<head><title>Moved Permanently</title></head>"
        "<body><h1>301 Moved Permanently</h1></body>"
        "</html>";
const char moved_temporarily[] =
        "<html>"
        "<head><title>Moved Temporarily</title></head>"
        "<body><h1>302 Moved Temporarily</h1></body>"
        "</html>";
const char not_modified[] =
        "<html>"
        "<head><title>Not Modified</title></head>"
        "<body><h1>304 Not Modified</h1></body>"
        "</html>";
const char bad_request[] =
        "<html>"
        "<head><title>Bad Request</title></head>"
        "<body><h1>400 Bad Request</h1></body>"
        "</html>";
const char unauthorized[] =
        "<html>"
        "<head><title>Unauthorized</title></head>"
        "<body><h1>401 Unauthorized</h1></body>"
        "</html>";
const char forbidden[] =
        "<html>"
        "<head><title>Forbidden</title></head>"
        "<body><h1>403 Forbidden</h1></body>"
        "</html>";
const char not_found[] =
        "<html>"
        "<head><title>Not Found</title></head>"
        "<body><h1>404 Not Found</h1></body>"
        "</html>";
const char internal_server_error[] =
        "<html>"
        "<head><title>Internal Server Error</title></head>"
        "<body><h1>500 Internal Server Error</h1></body>"
        "</html>";
const char not_implemented[] =
        "<html>"
        "<head><title>Not Implemented</title></head>"
        "<body><h1>501 Not Implemented</h1></body>"
        "</html>";
const char bad_gateway[] =
        "<html>"
        "<head><title>Bad Gateway</title></head>"
        "<body><h1>502 Bad Gateway</h1></body>"
        "</html>";
const char service_unavailable[] =
        "<html>"
        "<head><title>Service Unavailable</title></head>"
        "<body><h1>503 Service Unavailable</h1></body>"
        "</html>";

std::string to_string(Response::Status status)
{
    switch (status)
    {
    case Response::ok:
        return ok;
    case Response::created:
        return created;
    case Response::accepted:
        return accepted;
    case Response::no_content:
        return no_content;
    case Response::multiple_choices:
        return multiple_choices;
    case Response::moved_permanently:
        return moved_permanently;
    case Response::moved_temporarily:
        return moved_temporarily;
    case Response::not_modified:
        return not_modified;
    case Response::bad_request:
        return bad_request;
    case Response::unauthorized:
        return unauthorized;
    case Response::forbidden:
        return forbidden;
    case Response::not_found:
        return not_found;
    case Response::internal_server_error:
        return internal_server_error;
    case Response::not_implemented:
        return not_implemented;
    case Response::bad_gateway:
        return bad_gateway;
    case Response::service_unavailable:
        return service_unavailable;
    default:
        return internal_server_error;
    }
}

} // namespace stock_replies

string Response::getHeaderAttribute(const string &key, const string &def) const
{
    auto it = headers_.find(key) ;
    return it == headers_.end() ? def : it->second ;
}

void Response::stockReply(Response::Status status)
{
    status_ = status;
    content_.assign(stock_replies::to_string(status));
    setContentType("text/html");
    setContentLength() ;
}

static void gmt_time_string(char *buf, size_t buf_len, time_t *t) {
    strftime(buf, buf_len, "%a, %d %b %Y %H:%M:%S GMT", gmtime(t));
}

void Response::encodeFileData(const std::string &bytes, const std::string &encoding, const std::string &mime, time_t mod_time)
{
    status_ = ok ;

    if ( bytes.empty() ) return ;

    if ( encoding.empty() ) // try gzip encoding
    {
        if ( bytes.size() > 2 && bytes[0] == 0x1f && bytes[1] == 0x8b )
            addHeader("Content-Encoding", "gzip") ;
    }
    else
        addHeader("Content-Encoding", encoding) ;

    if ( !mime.empty() )
        addHeader("Content-Type", mime) ;

    addHeader("Access-Control-Allow-Origin", "*") ;

    char ctime_buf[64], mtime_buf[64] ;
    time_t curtime = time(nullptr) ;

    gmt_time_string(ctime_buf, sizeof(ctime_buf), &curtime);
    gmt_time_string(mtime_buf, sizeof(mtime_buf), &mod_time);

    addHeader("Date", ctime_buf) ;
    addHeader("Last-Modified", mtime_buf) ;


    addHeader("Etag", std::to_string(mod_time)) ;
    addHeader("Content-Length", std::to_string(bytes.size())) ;

    content_.assign(bytes) ;
}

std::map<string, string> well_known_mime_types {
#include "well_known_mime_types.hpp"
} ;

#ifndef _WIN32
#define stricmp strcasecmp
#define strnicmp strncasecmp
#endif

static string get_file_mime(const string &mime,  const std::string &p)
{
    if ( !mime.empty() ) return mime ;

    string extension ;
    size_t pos = p.find_last_of('.') ;
    if ( pos != string::npos ) {
        extension = p.substr(pos) ;
        if ( extension == ".gz" ) {
            string stem = p.substr(0, pos) ;
            size_t epos = stem.find_last_of('.') ;
            if ( epos != string::npos ) {
                extension = stem.substr(epos) ;
            }
        }
    }

    if ( !extension.empty() )
    {
        auto it = well_known_mime_types.find(extension) ;
        if ( it != well_known_mime_types.end() ) return it->second ;
    }

    return "application/octet-stream" ;
}



void Response::encodeFile(const std::string &file_path, const std::string &encoding, const std::string &mime )
{
    if ( !fileExists(file_path) ) {
        throw HttpResponseException(Response::not_found) ;
        return ;
    }

    time_t mod_time = fileLastWriteTime(file_path);

    string bytes = readFileToString(file_path) ;

    string omime = mime.empty() ? get_file_mime(mime, file_path) : mime ;

    encodeFileData(bytes, encoding, omime, mod_time) ;

}

void Response::writeJSON(const string &obj)
{
    write(obj, "application/json") ;
}

void Response::write(const string &content, const string &mime)
{
    content_.assign(content) ;
    setContentType(mime) ;
    setContentLength() ;
    setStatus(ok) ;
}

void Response::setContentType(const string &mime) {
    addHeader("Content-Type", mime) ;
}

void Response::setContentLength() {
    addHeader("Content-Length", to_string(content_.size())) ;
}

void Response::compress() {

    ostringstream compressed(ios::binary) ;

    {
        ozstream zstrm(compressed) ;
        zstrm.write(content_.data(), content_.size()) ;
    }

    content_.assign(compressed.str()) ;
    addHeader("Content-Encoding", "gzip") ;
    setContentLength();
}

string Response::toString() const {
    ostringstream strm ;
    if ( status_ == Response::ok )
        strm << status_ << " " << getHeaderAttribute("Content-Length", "0") ;
     else
        strm << status_ ;

    return strm.str() ;
}

static regex gzip_include_mime_rx("(text/.*)|(application/x-javascript.*)|(application/javascript)|(application/xhtml+xml)|(application/xml)") ;
static const size_t gzip_min_content_length = 200 ;

bool Response::contentBenefitsFromCompression() {
    size_t content_len = content_.size() ;
    if ( content_len < gzip_min_content_length ) return false ;

    string encoding = getHeaderAttribute("Content-Encoding") ;
    if ( encoding == "gzip" ) return false ;

    string mime = getHeaderAttribute("Content-Type") ;
    if ( mime.empty() ) return false ;

    return  ( regex_match(mime,  gzip_include_mime_rx) ) ;
}


void Response::append(const string &content) {
    content_.append(content) ;
}

bool Response::serveStaticFile(const string &folder, const string &path) {
    string p(folder + '/' + path) ;

    if ( fileExists(p) ) {
        encodeFile(p);
        return true ;
    }

    return false ;
}

void Response::setCookie(const string &name, const string &value, time_t expires, const string &path, const string &domain, bool secure, bool http_only)
{
    string cookie = name + '=' + value ;
    if ( expires > 0 ) {
        char etime_buf[64] ;
        gmt_time_string(etime_buf, sizeof(etime_buf), &expires);
        cookie += "; Expires=" ; cookie += etime_buf ;
    }

    if ( !path.empty() )
        cookie += "; Path=" + path ;

    if ( !domain.empty() )
        cookie += "; Domain=" + domain ;

    if ( secure )
        cookie += "; Secure" ;

    if ( http_only )
        cookie += "; HttpOnly" ;

    addHeader("Set-Cookie", cookie) ;

}

void Response::addHeader(const string &key, const string &val) {
    headers_[key] = val ;
}


} // namespace ws
