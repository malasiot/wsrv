#include <wsrv/mailer.hpp>

#include <fstream>

using namespace std ;
using namespace ws ;

int main(int argc, char *argv[]) {

    SMTPMailer c("smtp.gmail.com", 587) ;
    c.authenticate(argv[1], argv[2], ws::SMTPMailer::auth_method_t::START_TLS) ;

    SMTPMessage msg ;
    msg.setFrom(MailAddress("malasiot@iti.gr")) ;
    msg.addRecipient(MailAddress("malasiot@gmail.com")) ;
    msg.setSubject("test") ;
 //   msg.setBody("Hello\r\n<b>Καλημέρα</b>", true) ;

    ifstream strm1("/home/malasiot/Downloads/track-30-04-2020.gpx") ;

    msg.addAttachment(strm1, "track-30-04-2020.gpx", "application/gpx+xml") ;

    ifstream strm2("/home/malasiot/Downloads/track-28-04-2020.gpx") ;

    msg.addAttachment(strm2, "track-28-04-2020.gpx", "application/gpx+xml") ;

    c.submit(msg) ;
}
