#pragma once

#include <regex>
#include <stdexcept>
#include <functional>
#include <map>

#include <wsrv/forms/form_builder.hpp>

namespace wsrv {

class FormFieldNormalizers {
public:

    static Variant toBoolean(const Variant &val) ;
    static Variant toInteger(const Variant &val) ; 
    static Variant trim(const Variant &val) ; 
};

}

