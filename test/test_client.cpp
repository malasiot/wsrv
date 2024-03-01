#include <wsrv/http_client.hpp>

#include <iostream>

using namespace std ;
using namespace ws ;


int main(int argc, char *argv[]) {

    HTTPClient client ;


    HTTPClientRequest req("https://postman-echo.com/post?key=tt") ;
    req
            .setMethod(HTTPClientRequest::POST)
            .setBodyURLEncoded({{"name", "sotiris"}, {"data", "test"}}) ;

    auto res = client.execute(req) ;

    cout << res.content() << endl ;
}
