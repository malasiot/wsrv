#include <wsrv/server.hpp>
#include <wsrv/request_handler.hpp>
#include <wsrv/session.hpp>
#include <wsrv/exceptions.hpp>
#include <wsrv/sqlite3_session_manager.hpp>
#include <wsrv/middleware.hpp>


#include <wsrv/application.hpp>

#include <mutex>
#include <iostream>
#include <regex>


using namespace ws ;
using namespace std ;

static const std::string UPLOAD_FORM = R"(
<html><body>
<h1>Upload a file</h1>
<h3>{{error}}</h3>
<form method='POST' enctype='multipart/form-data'>
<input type='file' name='file' />
<input type='submit' value='Upload' />
</form>
</body></html>
)";

int main(int argc, char *argv[]) {


    HttpServer server("127.0.0.1:5110") ;

    Application *app = new Application() ;
    
    server.setHandler(app) ;

    SessionManager *session_manager = new SQLite3SessionManager("/tmp/session.sqlite");
  
    app->addRoute("GET|POST", "/upload", [session_manager](HTTPServerRequest& req, HTTPServerResponse& resp) {
         Session session(*session_manager, req, resp) ;
  
         if ( req.getMethod() == "POST"  ) {
             auto files = req.getUploadedFiles() ;
             if ( files.empty() 
                  || files.find("file") == files.end() 
                  || files["file"].size_ == 0 ) {
                        resp.write(std::regex_replace(UPLOAD_FORM, std::regex("{{error}}"), "No file uploaded")) ;
                 return ;
             } else {
                auto &file = files["file"] ;
                cout << "Received file: " << file.name_ << " (" << file.size_ << " bytes)\n" ;
                resp.write("File uploaded successfully: " + file.name_) ;
             }

            
         }
         else {
             resp.write(std::regex_replace(UPLOAD_FORM, std::regex("{{error}}"), "")) ;
         }
         
    }) ;
  
    
    server.run() ;
}
