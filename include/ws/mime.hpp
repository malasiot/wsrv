#ifndef WS_MIME_HPP
#define WS_MIME_HPP

#include <string>
#include <vector>

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

    void setBoundary(const std::string &b) { boundary_ = b ; }
    const std::string & boundary() const { return boundary_ ; }

    std::string format() const ;

    virtual std::string encode() const { return content_ ; }

protected:

    void encodeRaw(const std::string &text) ;

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

class MimeMultiPart: public MimePart {
public:
    enum MultiPartType {
        Mixed           = 0,            // RFC 2046, section 5.1.3
        Digest          = 1,            // RFC 2046, section 5.1.5
        Alternative     = 2,            // RFC 2046, section 5.1.4
        Related         = 3,            // RFC 2387
        Report          = 4,            // RFC 6522
        Signed          = 5,            // RFC 1847, section 2.1
        Encrypted       = 6             // RFC 1847, section 2.2
    };

    MimeMultiPart(const MultiPartType type = Related);

    void addPart(MimePart *part) { parts_.push_back(part) ; }

    const std::vector<MimePart *> parts() const { return parts_ ; }

    std::string encode() const override ;

private:

    std::vector<MimePart *> parts_ ;
    MultiPartType mp_type_ ;
};

}

#endif
