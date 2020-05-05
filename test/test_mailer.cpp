#include <ws/mailer.hpp>

#include <fstream>

using namespace std ;
using namespace ws ;

int main(int argc, char *argv[]) {

    SMTPMailer c("smtp.gmail.com", 587) ;
    c.authenticate("malasiot@gmail.com", "partita96", ws::SMTPMailer::auth_method_t::START_TLS) ;

    SMTPMessage msg ;
    msg.setFrom(MailAddress("malasiot@iti.gr")) ;
    msg.addRecipient(MailAddress("malasiot@gmail.com")) ;
    msg.setSubject("test") ;

    MimeHtml text("Hello\r\n<b>Καλημέρα</b>") ;
  //  msg.setContent(&text) ;

    ifstream strm("/home/malasiot/Downloads/track-02-05-2020.gpx") ;
    MimeFileInline file(strm, "track-02-05-2020.gpx", "application/gpx+xml") ;
    msg.setContent(&file) ;

    c.submit(msg) ;
}
