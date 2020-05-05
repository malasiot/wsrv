#ifndef WS_SMTP_SOCKET_HPP
#define WS_SMTP_SOCKET_HPP

#include <asio/ip/tcp.hpp>
#include <asio/read_until.hpp>
#include <asio/connect.hpp>
#include <asio/ssl.hpp>
#include <asio/streambuf.hpp>
#include <asio/deadline_timer.hpp>

namespace ws { namespace detail {

class Socket {
public:
    Socket(const std::string& hostname, unsigned port) ;

    template<typename Socket>
    void send(Socket& socket, const std::string& line)
    {
        try
        {
            std::string l = line + "\r\n";
            write(socket, asio::buffer(l, l.size()));
        }
        catch (asio::system_error&)
        {

        }
    }


    template<typename Socket>
    std::string receive(Socket& socket, bool raw)
    {
        try
        {
            asio::read_until(socket, *strmbuf_, "\n");
            std::string line;
            getline(*istrm_, line, '\n');
            //     if (!raw)
            //          trim_if(line, is_any_of("\r\n"));
            return line;
        }
        catch (asio::system_error&)
        {
            //           throw dialog_error("Network receiving error.");
        }
    }

    virtual void send(const std::string &line) {
        send(socket_, line) ;
    }

    virtual std::string receive(bool raw = false) {
        return receive(socket_, raw) ;
    }


    Socket(Socket &&other);
    virtual ~Socket() ;

protected:

    const std::string hostname_;
    const unsigned int port_ ;

    std::unique_ptr<asio::io_context> ios_;
    asio::ip::tcp::socket socket_;
    std::unique_ptr<asio::streambuf> strmbuf_;
    std::unique_ptr<std::istream> istrm_;

};


class SSLSocket: public Socket {
public:
    SSLSocket(Socket &&s) ;

    void send(const std::string &line) {
        Socket::send(ssl_socket_, line) ;
    }

    std::string receive(bool raw = false) {
        return Socket::receive(ssl_socket_, raw) ;
    }
private:

    asio::ssl::context context_;

    asio::ssl::stream<asio::ip::tcp::socket&> ssl_socket_;

};

} // detail

} // ws



#endif
