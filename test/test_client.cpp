#include <ws/client.hpp>

using namespace std ;
using namespace ws ;


int main(int argc, char *argv[]) {

    HttpClient client ;
    client.setHost("vision.iti.gr") ;
    client.get("http://www.hellaspath.gr/");
}
