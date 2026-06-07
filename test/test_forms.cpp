#include <wsrv/server.hpp>
#include <wsrv/sqlite3_session_manager.hpp>
#include <wsrv/middleware.hpp>
#include <wsrv/application.hpp>
#include <wsrv/forms/form_builder.hpp>
#include <wsrv/forms/validators.hpp>
#include <wsrv/middleware/locale.hpp>
#include <twig/loader.hpp>

#include <mutex>
#include <iostream>

#include <twig/renderer.hpp>
#include <twig/translator.hpp>
#include <wsrv/blueprint.hpp>

using namespace ws ;
using namespace std ;
using twig::tr ;

string FORM_TEMPLATE = R"(

    {% extends "page.html.twig" %}

   
    {% block content %}
         {{forms.form(form)}}
    {% endblock %}
    )";

int main(int argc, char *argv[]) {
    HttpServer server("127.0.0.1:5110") ;

    Application app ;
    
    server.setHandler(&app) ;

    std::shared_ptr<twig::FileSystemTemplateLoader> loader(new twig::FileSystemTemplateLoader({std::string(DATA_DIR) + "/templates/"}));
    twig::TemplateRenderer rdr(loader) ;

    twig::TranslationManager translator ;
    translator.loadAllFromDirectory(std::string(DATA_DIR) + "/translations/");

    rdr.setTranslationManager(&translator) ;

    Form form("/form", "POST") ;

    form.field<TextField>("username")
        .placeholder(tr("Write user name"))
        .label(tr("Username"))
        .attribute("required", true)
        .addClass("grid").addClass("flex")
        .addValidator(FormFieldValidators::required())
        .addValidator(FormFieldValidators::matches(std::regex("[a-zA-Z_]+"))) ;

    form.field<SelectField>("role")
        .addOption("admin", tr("admin"))
        .addOption("guest", tr("guest"))
        .label(tr("Role"));
  
    app.addRoute("GET|POST", "/form", [&rdr, &form, &translator](HTTPServerRequest& req, HTTPServerResponse& resp) {
       
        auto locale = req.data().get<LocaleResolverData>() ;
        if ( req.getMethod() == "POST" && form.process(req.getPostAttributes()) ) {
            resp.write(translator.translate("welcome", locale->locale_, Variant::Object{{"name", form.getValue("username")}})) ;
            return ;
         } 
           
        try {
            auto locale = req.data().get<LocaleResolverData>() ;
             resp.write(rdr.render("test_form.html", Variant::Object{{"form", form.render(&translator, locale->locale_)}})) ;
        } catch ( std::exception &e ) {
             cout << e.what() << endl ;
        }
        

        
    }, {std::make_shared<LocaleResolver>(translator.getSupportedLocales())})  ;
    
    server.run() ;
}
