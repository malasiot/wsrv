#include <wsrv/server.hpp>
#include <wsrv/request_handler.hpp>
#include <wsrv/session.hpp>
#include <wsrv/exceptions.hpp>
#include <wsrv/sqlite3_session_manager.hpp>
#include <wsrv/middleware.hpp>
#include <wsrv/application.hpp>
#include <wsrv/middleware/locale.hpp>
#include <wsrv/middleware/request_logger.hpp>
#include <mutex>
#include <iostream>

#include <twig/renderer.hpp>

using namespace ws ;
using namespace std ;



int main(int argc, char *argv[]) {

    HttpServer server("127.0.0.1:5110") ;

    const string root = "/home/malasiot/source/ws/data/routes/" ;

    Application *app = new Application() ;
    
    server.setHandler(app) ;

    SessionManager *session_manager = new SQLite3SessionManager("/tmp/session.sqlite");
    twig::TemplateRenderer rdr(nullptr) ;

    Blueprint bp("user") ;

    DebugLogger logger ;
  
    app->useGlobal(std::make_shared<RequestLogger>(logger)) ;

    bp.addRoute("GET", "/{id:\\d+}/{action:show|hide}", [session_manager, &rdr](HTTPServerRequest& req, HTTPServerResponse& resp) {
         Session session(*session_manager, req, resp) ;

         auto locale_data = req.data().get<LocaleResolverData>() ;
  
         resp.write("hello " +  req.getRouteAttribute("id") + " " + (locale_data ? locale_data->locale_ : "") ) ;

         if ( session.contains("id") )
            resp.append(", your session id is: " + session.get("id")) ;
         else
            session.add("id", req.getRouteAttribute("id")) ;
    }, { std::make_shared<LocaleResolver>(std::vector<std::string>{"en", "el"}, "en")}) ;
    app->addRoute("GET", "/static/{file:.*}", [session_manager, &rdr](HTTPServerRequest& req, HTTPServerResponse& resp) {
     //   resp.write(rdr.renderString("Requested file: {{file}}", { {"file", req.getRouteAttribute("file")} })) ;
      
        if ( !resp.serveStaticFile("/Users/malasiot/source/wsrv", req.getRouteAttribute("file")) ) {
             resp = HTTPServerResponse::stockReply(HTTPServerResponse::not_found) ;
        }
    }) ;

    app->registerBlueprint(bp) ;
    
    server.run() ;
}
