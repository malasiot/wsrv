#include "smtp_socket.hpp"

using namespace std ;
using namespace asio ;

namespace ws {
namespace detail {

Socket::Socket(const string &hostname, unsigned port):
    hostname_(hostname), port_(port), ios_(new io_context()), socket_(*ios_), strmbuf_(new asio::streambuf()), istrm_(new istream(strmbuf_.get())) {

    try {
        ip::tcp::resolver res(*ios_);
        asio::connect(socket_, res.resolve(hostname_, to_string(port_)));
    } catch ( system_error &e ) {

    }
}

Socket::Socket(Socket&& other) : hostname_(move(other.hostname_)), port_(other.port_), socket_(move(other.socket_)) {
    ios_.reset(other.ios_.release());
    strmbuf_.reset(other.strmbuf_.release());
    istrm_.reset(other.istrm_.release());
}

Socket::~Socket()
{
    try
    {
        socket_.close();
    }
    catch (...)
    {
    }
}

SSLSocket::SSLSocket(Socket &&s): Socket(move(s)), context_(asio::ssl::context::sslv23), ssl_socket_(Socket::socket_, context_) {
    try {
        ssl_socket_.set_verify_mode(asio::ssl::verify_none);
        ssl_socket_.handshake(asio::ssl::stream_base::client);
    }
    catch ( system_error & ){

    }
}

}}
