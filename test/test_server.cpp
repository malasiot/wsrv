#include <ws/server.hpp>
#include <ws/request_handler.hpp>
#include <ws/session.hpp>
#include <ws/exceptions.hpp>

using namespace ws ;
using namespace std ;



class App: public RequestHandler {
public:

    App(const std::string &root_dir):
        root_(root_dir)
    {


    }

    void handle(const Request &req, Response &resp) override {

        if ( req.matches("GET", "/map/") ) {

            resp.write("map") ;
            return ;
        }


        throw HttpResponseException(Response::not_found) ;
    }


private:

    string root_ ;
};


#define __(S) boost::locale::translate(S)

int main(int argc, char *argv[]) {


     Server server("127.0.0.1", "5000") ;

    const string root = "/home/malasiot/source/ws/data/routes/" ;
    std::unique_ptr<App> service(new App(root)) ;

    server.setHandler(service.get()) ;

//    server.addFilter(new RequestLoggerFilter(logger)) ;
//    server.addFilter(new GZipFilter()) ;

    server.run() ;
}
