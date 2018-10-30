**Simple HTTP server based on asio**

Syntax:

```
Server server("<address>", "<port>") ;

Application *service = new Application() ;

server.setHandler(service) ;

server.run() ;
```
The service `Application` should be an instance of RequestHandler:
```
class Application: public RequestHandler {
public:

    Application() ; // do all one-time initialization here

    // called for every server request. Fill the response object or throw a HttpResponseException
    
    void handle(const Request &req, Response &resp) override ;
};
```

`Request` contains dictionaries of parsed HTTP variables similar to PHP. `SERVER_` contains server related parameters, `GET_` contains query parameters, `POST_` data submitted via POST, `COOKIES_` contains client cookies and `FILE_` all uploaded files. 

The `Response` variables `headers_`, `content_` and `status_` have to be filled in for a valid request. Normally you will use one of the helper functions such as `write(<content_string>, <mime>)` or `encode_file(<file_path>)`. If the request cannot be handled you should throw a `HttpResponseException` using the appropriate status id e.g. `throw HttpResponseException(Response::not_found)`.

# Routing

Request routing is performed by matching request url to a routing pattern. This is performed by the Route class that parses the url pattern and performs the matching with an input url.
```C++
Route user("/user/{id:\d+}/{action:show|hide}/") ;

Dictionary params ;
bool res = user.matches("/user/2/show/", params) ;
```
The above will return `true` and set `params["id"] = "2"`, `params["action"] = "show"`.

The route pattern is of the form `/<pat1>/<pat2>/<pat3> ... /<patn>/` where each sub-pattern has the format
`({[<param key>][:<regex>]}|<characters>)[?]`. Use a `?` after a sub-pattern to indicate that the element (and the subsequent elements) is optional. Leading and trailing slashes are ignored.

For convenience the `Request` class defines a `matches()` function that will check if the request method matches and the request path matches a Route pattern:
```
Request req ;
Dictionary params ;
UserController user ;

if ( req.matches("GET", "/user/{id:\d+}/show/", params) ) user.show(params["id"]) ;
```

Reverse routing is performed by calling `url()` member function of a route passing a dictionary of key/value pairs which should correspond to the named route parameters.
