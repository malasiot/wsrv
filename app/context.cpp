#include "context.hpp"

using namespace ws ;

AppContext::AppContext(const Variant &config) {
    web_root_ = config["root"].toString() ;

    session_manager_.reset(new SQLite3SessionManager(config["session.path"].toString()));
    loader_.reset(new twig::FileSystemTemplateLoader({web_root_ + "/templates/"}));
    rdr_.reset(new twig::TemplateRenderer(loader_)) ;
    translator_.reset(new twig::TranslationManager()) ;
    translator_->loadAllFromDirectory(web_root_ + "/translations/");
    rdr_->setTranslationManager(translator_.get());

    locales_ = config["language.locales"].toArray() ;
}