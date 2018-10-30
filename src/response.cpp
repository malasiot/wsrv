
#include <ws/response.hpp>
#include <ws/zstream.hpp>

#include <ws/exceptions.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include <boost/asio.hpp>
#include <boost/date_time.hpp>
#include <boost/regex.hpp>

#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <time.h>

#include "detail/filesystem.hpp"

using namespace std ;
namespace fs = boost::filesystem ;

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

boost::asio::const_buffer to_buffer(Response::Status status)
{
    switch (status)
    {
    case Response::ok:
        return boost::asio::buffer(ok);
    case Response::created:
        return boost::asio::buffer(created);
    case Response::accepted:
        return boost::asio::buffer(accepted);
    case Response::no_content:
        return boost::asio::buffer(no_content);
    case Response::multiple_choices:
        return boost::asio::buffer(multiple_choices);
    case Response::moved_permanently:
        return boost::asio::buffer(moved_permanently);
    case Response::moved_temporarily:
        return boost::asio::buffer(moved_temporarily);
    case Response::not_modified:
        return boost::asio::buffer(not_modified);
    case Response::bad_request:
        return boost::asio::buffer(bad_request);
    case Response::unauthorized:
        return boost::asio::buffer(unauthorized);
    case Response::forbidden:
        return boost::asio::buffer(forbidden);
    case Response::not_found:
        return boost::asio::buffer(not_found);
    case Response::internal_server_error:
        return boost::asio::buffer(internal_server_error);
    case Response::not_implemented:
        return boost::asio::buffer(not_implemented);
    case Response::bad_gateway:
        return boost::asio::buffer(bad_gateway);
    case Response::service_unavailable:
        return boost::asio::buffer(service_unavailable);
    default:
        return boost::asio::buffer(internal_server_error);
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


std::vector<boost::asio::const_buffer> response_to_buffers(Response &rep, bool is_head)
{
    std::vector<boost::asio::const_buffer> buffers;
    buffers.push_back(status_strings::to_buffer(rep.status_));

    for( const auto &h: rep.headers_ )
    {
        buffers.push_back(boost::asio::buffer(h.first));
        buffers.push_back(boost::asio::buffer(misc_strings::name_value_separator));
        buffers.push_back(boost::asio::buffer(h.second));
        buffers.push_back(boost::asio::buffer(misc_strings::crlf));
    }
    if ( !is_head ) {
        buffers.push_back(boost::asio::buffer(misc_strings::crlf));
        buffers.push_back(boost::asio::buffer(rep.content_));
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
            headers_.add("Content-Encoding", "gzip") ;
    }
    else
        headers_.add("Content-Encoding", encoding) ;

    if ( !mime.empty() )
        headers_.add("Content-Type", mime) ;

    headers_.add("Access-Control-Allow-Origin", "*") ;

    char ctime_buf[64], mtime_buf[64] ;
    time_t curtime = time(nullptr) ;

    gmt_time_string(ctime_buf, sizeof(ctime_buf), &curtime);
    gmt_time_string(mtime_buf, sizeof(mtime_buf), &mod_time);

    headers_.add("Date", ctime_buf) ;
    headers_.add("Last-Modified", mtime_buf) ;


    headers_.add("Etag", boost::lexical_cast<std::string>(mod_time)) ;
    headers_.add("Content-Length", boost::lexical_cast<std::string>(bytes.size())) ;

    content_.assign(bytes) ;
}

std::map<string, string> well_known_mime_types {
#include "well_known_mime_types.hpp"
} ;

#ifndef _WIN32
#define stricmp strcasecmp
#define strnicmp strncasecmp
#endif

static string get_file_mime(const string &mime,  const boost::filesystem::path &p)
{
    if ( !mime.empty() ) return mime ;

    string extension = p.extension().string() ;

    if ( extension == ".gz" )
        extension = p.stem().extension().string() ;

    if ( !extension.empty() )
    {
        auto it = well_known_mime_types.find(extension) ;
        if ( it != well_known_mime_types.end() ) return it->second ;
    }

    return "application/octet-stream" ;
}



void Response::encodeFile(const std::string &file_path, const std::string &encoding, const std::string &mime )
{
    if ( !fs::exists(file_path) ) {
        throw HttpResponseException(Response::not_found) ;
        return ;
    }

    time_t mod_time = boost::filesystem::last_write_time(file_path);

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
    headers_.replace("Content-Type", mime) ;
}

void Response::setContentLength() {
    headers_.replace("Content-Length", to_string(content_.size())) ;
}

void Response::append(const string &content)
{
    content_.append(content) ;
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

    headers_.add("Set-Cookie", cookie) ;

}


} // namespace ws
