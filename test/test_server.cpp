#include <wsrv/server.hpp>
#include <wsrv/request_handler.hpp>
#include <wsrv/session.hpp>
#include <wsrv/exceptions.hpp>
#include <wsrv/sqlite3_session_manager.hpp>
#include <wsrv/middleware.hpp>


#include <wsrv/application.hpp>

#include <mutex>
#include <iostream>

using namespace ws ;
using namespace std ;


class RequestLogger: public IMiddleware {
public:
    void handle(HTTPServerRequest& req, HTTPServerResponse& res, PipelineContext& ctx) {
        std::cout << "[Log] " << req.toString() << " -> " ;
        ctx.next(req, res); // Move to next layer
        std::cout <<  res.toString() << "\n";
    }
};

class GZipFilter: public IMiddleware {
public:

    void handle(HTTPServerRequest& req, HTTPServerResponse& res, PipelineContext& ctx) {
        ctx.next(req, res); // Move to next layer

        if ( req.supportsGzip() && res.contentBenefitsFromCompression() )
            res.compress();
    }
};



int main(int argc, char *argv[]) {


    HttpServer server("127.0.0.1:5110") ;

    const string root = "/home/malasiot/source/ws/data/routes/" ;

    Application *app = new Application() ;
    
    server.setHandler(app) ;

    SessionManager *session_manager = new SQLite3SessionManager("/tmp/session.sqlite");
  
    app->useGlobal(std::make_shared<RequestLogger>()) ;
    app->useGlobal(std::make_shared<GZipFilter>()) ;
    app->addRoute("GET", "/user/{id:\\d+}/{action:show|hide}", [session_manager](HTTPServerRequest& req, HTTPServerResponse& resp) {
         Session session(*session_manager, req, resp) ;
  
         resp.write("hello " + req.getRouteAttribute("id")) ;

         if ( session.contains("id") )
            resp.append(", your session id is " + session.get("id")) ;
         else
            session.add("id", req.getRouteAttribute("id")) ;
    }) ;

    server.run() ;
}
