#include <ws/mime.hpp>

#include <iostream>
#include <sstream>

#include "base64.hpp"

using namespace std ;

namespace ws {

// https://github.com/bluetiger9/SmtpClient-for-Qt/blob/v1.1/src/mimecontentformatter.cpp

string MimePart::format() const {
   string res ;

   // Header

   res.append("Content-Type: ").append(type_);

   if ( !name_.empty() )
      res.append("; name=\"").append(name_).append("\"");

   if ( !charset_.empty() )
      res.append("; charset=").append(charset_);

   if ( boundary_.empty() )
        res.append("; boundary=").append(boundary_);

    res.append("\r\n");

    res.append("Content-Transfer-Encoding: ");

    switch ( encoding_ ) {
        case SevenBit:
            res.append("7bit\r\n");
            break;
        case EightBit:
            res.append("8bit\r\n");
            break;
        case Base64:
            res.append("base64\r\n");
            break;
        case QuotedPrintable:
            res.append("quoted-printable\r\n");
            break;
     }

    if ( !id_.empty() )
        res.append("Content-ID: <").append(id_).append(">\r\n");

    res.append(header_).append("\r\n");

    // Content
    res.append(content_) ;

    res.append("\r\n");

    return res ;
}

void MimePart::encodeRaw(const string &text) {
    switch ( encoding_ ) {
    case EightBit:
        content_ = text ;
        break ;
    case Base64:
        content_ = base64_encode(text) ;
        break ;
    }
}

MimePlainText::MimePlainText(const std::string &text) {
    setContentType("text/plain") ;
    setCharset("utf-8") ;
    setEncoding(EightBit);

    encodeRaw(text) ;
}


MimeHtml::MimeHtml(const string &text): MimePlainText(text) {
    setContentType("text/html") ;
}

MimeFile::MimeFile(istream &strm, const string &file_name, const string &ctype) {
    setEncoding(Base64);
    setContentType(ctype);
    setContentName(file_name) ;

    stringstream ss;
    ss << strm.rdbuf();
    encodeRaw(ss.str());
}

MimeFileInline::MimeFileInline(istream &strm, const string &file_name, const string &mime): MimeFile(strm, file_name, mime) {
    addHeaderLine("Content-Disposition: inline") ;
}





}
