#include <ws/mailer.hpp>

int main(int argc, char *argv[]) {

    ws::SMTPMailer c("smtp.gmail.com", 587) ;
    c.authenticate("malasiot@gmail.com", "partita96", ws::SMTPMailer::auth_method_t::START_TLS) ;
}
