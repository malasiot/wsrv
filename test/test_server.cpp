#include <ws/server.hpp>
#include <ws/request_handler.hpp>
#include <ws/session.hpp>
#include <ws/exceptions.hpp>
#include <ws/fs_session_handler.hpp>
#include <ws/logger.hpp>
#include <ws/filters/request_logger.hpp>
#include <ws/filters/static_file_handler.hpp>

#include <mutex>
#include <iostream>

using namespace ws ;
using namespace std ;

class SimpleLogger: public Logger {
public:
    void log(Severity s, const string &msg) override {
        unique_lock<mutex> lock(lock_) ;

        if ( s == Logger::Error )
            cerr << msg << endl ;
        else
            cout << msg << endl ;
    }

    mutex lock_ ;
};

class App: public RequestHandler {
public:

    App(const std::string &root_dir):
        root_(root_dir), session_handler_("/tmp/session.sqlite")
    {

    }

    void handle(const Request &req, Response &resp) override {

        Session session(session_handler_, req, resp) ;

        Dictionary attrs ;
        if ( req.matches("GET", R"(/user/{id:\d+}/{action:show|hide}/)", attrs) ) {

            resp.write("hello " + attrs.get("id")) ;

            session.data().add("id", attrs.get("id")) ;
            return ;
        }


        throw HttpResponseException(Response::not_found) ;
    }

private:

    FileSystemSessionHandler session_handler_ ;
    string root_ ;
};


int main(int argc, char *argv[]) {


     Server server("127.0.0.1", "5000") ;

    const string root = "/home/malasiot/source/ws/data/routes/" ;

    server.setHandler(new App(root)) ;

    unique_ptr<SimpleLogger> logger(new SimpleLogger()) ;

    server.addFilter(new StaticFileHandler(root)) ;
    server.addFilter(new RequestLoggerFilter(logger.get())) ;

    server.run() ;
}
