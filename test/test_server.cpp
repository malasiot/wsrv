#include <wsrv/server.hpp>
#include <wsrv/request_handler.hpp>
#include <wsrv/session.hpp>
#include <wsrv/exceptions.hpp>
#include <wsrv/fs_session_manager.hpp>

#include <mutex>
#include <iostream>

using namespace ws ;
using namespace std ;

// You may implement a filter like handler like this. Here we put the task in the destructor so that it is performed after the app
// has the chance to fill the response

class RequestLogger {
public:
    RequestLogger(const Request &req, const Response &res): req_(req), res_(res) {}
    ~RequestLogger() {
        unique_lock<mutex> lock(lock_) ;

        if ( res_.getStatus() == Response::ok ) {
            cout << req_.toString() << " " << res_.toString() << endl ;
        } else {
            cerr << req_.toString() << " " << res_.toString() << endl ;
        }
    }

    const Request &req_ ;
    const Response &res_ ;
    mutex lock_ ;
};

class GZipFilter {
public:
    GZipFilter(const Request &req, Response &res): req_(req), res_(res) {}
    ~GZipFilter() {
        if ( req_.supportsGzip() && res_.contentBenefitsFromCompression() )
            res_.compress();
    }

    const Request &req_ ;
    Response &res_ ;

};

class App: public RequestHandler {
public:

    using Dictionary = std::map<std::string, std::string> ;

    App(const std::string &root_dir):  root_(root_dir)
    {

    }

    void handle(const Request &req, Response &resp) override {

        RequestLogger logger(req, resp) ;
        GZipFilter gzip(req, resp) ;

        try {
            Session session(*session_manager_, req, resp) ;

            Dictionary attrs ;
            if ( req.matches("GET", R"(/user/{id:\d+}/{action:show|hide}/)", attrs) ) {

                resp.write("hello " + attrs["id"]) ;

                session.add("id", attrs["id"]) ;
                return ;
            } else if ( resp.serveStaticFile(root_, req.getPath()) ) {
                return ;
            } else {
                resp.stockReply(Response::not_found) ;
            }
        }
        catch ( std::runtime_error &e ) {
            resp.stockReply(Response::internal_server_error) ;
        }
    }

    void setSessionManager(SessionManager *sm) {
        session_manager_ = sm ;
    }
private:

    string root_ ;
    SessionManager *session_manager_ ;
};


int main(int argc, char *argv[]) {


    HttpServer server("127.0.0.1:5000") ;

    const string root = "/home/malasiot/source/ws/data/routes/" ;

    App *app = new App(root) ;
    server.setHandler(app) ;
    app->setSessionManager(new SQLite3SessionManager("/tmp/session.sqlite")) ;

    server.run() ;
}
