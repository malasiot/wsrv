#include <ws/filters/gzip_filter.hpp>
#include <ws/filter_chain.hpp>
#include <ws/zstream.hpp>
#include <regex>

using namespace std ;

namespace ws {

static regex gzip_include_mime_rx("(text/.*)|(application/x-javascript.*)|(application/xhtml+xml)|(application/xml)") ;
static const size_t gzip_min_content_length = 200 ;

bool content_benefits_from_compression(const string &mime) {
    if (regex_match(mime,  gzip_include_mime_rx) ) return true ;
    return false ;
}

void GZipFilter::handle(Request &req, Response &resp, FilterChain &chain)
{
    chain.next(req, resp) ;

    string saccept = req.SERVER_.get("Accept-Encoding") ;

    string encoding = resp.headers_.get("Content-Encoding") ;
    string mime = resp.headers_.get("Content-Type") ;

    if ( resp.status_ == Response::ok &&
         saccept.find("gzip") != string::npos &&
         encoding != "gzip" &&
         !resp.content_.empty() &&
         resp.content_.length() > gzip_min_content_length &&
         content_benefits_from_compression(mime) ) {

        ostringstream compressed(ios::binary) ;
        {
            ozstream zstrm(compressed) ;
            zstrm.write(&resp.content_[0], resp.content_.size()) ;
        }

        resp.content_.assign(compressed.str()) ;
        resp.headers_.replace("Content-Encoding", "gzip") ;
        resp.setContentLength();

    }

}

} // namespace ws
