#include <wsrv/client.hpp>

#include <iostream>

using namespace std ;
using namespace ws ;


int main(int argc, char *argv[]) {

    HTTPClient client ;
    client.setHost("vision.iti.gr") ;
    HTTPServerResponse res = client.post("https://postman-echo.com/post", {{"name", "ok"}, {"data", "hello"}});
    cout << res.content() << endl ;
}
