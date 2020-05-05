#ifndef WS_MIME_HPP
#define WS_MIME_HPP

#include <string>

namespace ws {

class MimePart {
public:
    enum Encoding { SevenBit, EightBit, Base64, QuotedPrintable } ;

    MimePart() = default ;

    const std::string& header() const { return header_ ; }

    void setHeader(const std::string & header) { header_ = header ; }

    void addHeaderLine(const std::string & line){ header_ += line + "\r\n" ; }

    void setContentId(const std::string & cId) { id_ = cId ; }
    const std::string & contentId() const { return id_ ; }

    void setContentName(const std::string & cName) { name_ = cName ; }
    const std::string & contentName() const { return name_ ; }

    void setContentType(const std::string & cType) { type_ = cType ; }
    const std::string & contentType() const { return type_ ; }

    void setCharset(const std::string & charset) { charset_ = charset ; }
    const std::string & charset() const { return charset_ ; }

    void setEncoding(Encoding enc) { encoding_ = enc ; }
    Encoding encoding() const { return encoding_ ; }

    std::string format() const ;

protected:

     void encodeRaw(const std::string &text) ;

private:

    std::string header_,  name_, id_, type_, charset_, boundary_, content_ ;
    Encoding encoding_ ;
};

class MimePlainText: public MimePart {
public:
    MimePlainText(const std::string &text) ;
};

class MimeHtml: public MimePlainText {
public:
     MimeHtml(const std::string &text) ;
};

class MimeFile: public MimePart {
public:
    MimeFile(std::istream &strm, const std::string &file_name, const std::string &mime = "application/octet-stream") ;
};

class MimeFileInline: public MimeFile {
public:
    MimeFileInline(std::istream &strm, const std::string &file_name, const std::string &mime = "application/octet-stream") ;
};

}

#endif
