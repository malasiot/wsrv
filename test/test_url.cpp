#include <wsrv/url.hpp>

#include <iostream>

using namespace ws ;
using namespace std ;

int main(int argc, char *argv) {
    cout << URL("https://djjd//Καλ/../jjdd/./index.html").normalizePath().str() << endl ;

    cout << URLBuilder("https://vision.iti.gr")
        .addPathSegment("test")
        .addQueryParam("q", "test")
        .build().str()  << endl ;
}
