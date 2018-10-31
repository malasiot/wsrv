#ifndef WS_LOGGER_HPP
#define WS_LOGGER_HPP

#include <string>

namespace ws {

//  abstract server logger

class Logger {
public:
    enum Severity { Info, Warning, Error, Fatal, Debug } ;
    virtual void log(Severity s, const std::string &msg) = 0 ;
} ;

class NullLogger: public Logger {
public:
    void log(Severity s, const std::string &msg) override {}

    static NullLogger &instance() {
        static NullLogger logger ;
        return logger ;
    }
};

}
#endif
