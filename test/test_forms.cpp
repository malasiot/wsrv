#include <wsrv/server.hpp>
#include <wsrv/sqlite3_session_manager.hpp>
#include <wsrv/middleware.hpp>
#include <wsrv/application.hpp>
#include <wsrv/forms/form_builder.hpp>
#include <wsrv/forms/validators.hpp>
#include <twig/loader.hpp>

#include <mutex>
#include <iostream>

#include <twig/renderer.hpp>

using namespace ws ;
using namespace std ;

string FORM_TEMPLATE2 = R"(
      <form action="{{ form.action }}" method="{{ form.method }}">
        {% for k,field in form.fields %}
        <div class="form-group {% if field.error is not empty %}has-error{% endif %}">
        
        {%if field.widget == 'select' %}
        <label>{{ field.label }}</label>
            <select name="{{ field.name }}" value="{{ field.value }}">
             {% for option in field.options %}
                    <option value="{{ option.value }}" {% if option.selected %}selected{% endif %}>{{ option.label }}</option>
                {% endfor %}
            </select>
              {% elif field.widget == "radio" %}
            <span class="group-label"><strong>{{ field.label }}</strong></span>
            <div class="radio-options-wrapper">
                {% for option in field.options %}
                    <label class="radio-inline">
                        <input type="radio" 
                               name="{{ field.name }}" 
                               value="{{ option.value }}"
                               {{ html_attr(option.attr) }}>
                        {{ option.label }}
                    </label>
                {% endfor %}
            </div>

        {# --- 2. SELECT DROP-DOWNS WIDGET --- #}
            {%elif field.widget == 'input'%}
                {% if field.type == "checkbox" %}
                <label>
                    <input type="checkbox" name="{{ field.name }}" {% if field.checked %}checked{% endif %} {{html_attr(field.attr)}}>
                    {{ field.label }}
                </label>
            {% else %}
                <label>{{ field.label }}</label>
                <input type="{{ field.type }}" name="{{ field.name }}" value="{{ field.value }}" {{html_attr(field.attr)}}>
            {% endif %}

            {%endif%}
       
      
        <span class="err">{{ field.error }}</span>
      
    </div>
    {% endfor %}
    <button type="submit">Register</button>
</form>
)";

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

    std::shared_ptr<twig::FileSystemTemplateLoader> loader(new twig::FileSystemTemplateLoader({"/Users/malasiot/source/wsrv/data/templates/"}));
    twig::TemplateRenderer rdr(loader) ;

    Form form("/form", "POST") ;

    form.field<TextField>("username")
        .label("Username")
        .attribute("required", true)
        .attribute("placeholder", "Write user name")
        .attribute("class", Variant::Array{"grid", "flex"})
        .addValidator(FormFieldValidators::required())
        .addValidator(FormFieldValidators::matches(std::regex("[a-zA-Z_]+"))) ;

    form.field<SelectField>("role")
        .addOption("admin", "Admin")
        .addOption("guest", "Guest")
        .label("Role");
  
    app.addRoute("GET|POST", "/form", [&rdr, &form](HTTPServerRequest& req, HTTPServerResponse& resp) {
       
        if ( req.getMethod() == "POST" && form.process(req.getPostAttributes()) ) {
            resp.write("Thank you") ;
            return ;
         } 
           
            try {
             resp.write(rdr.render("test_form.html", {{"form", form.render()}})) ;
            } catch ( std::exception &e ) {
                cout << e.what() << endl ;
            }
        

        
    }) ;
    
    server.run() ;
}
