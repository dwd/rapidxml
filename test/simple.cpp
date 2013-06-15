#include "rapidxml.hpp"
#include <string>
#include <cassert>
#include <iostream>

int main(int argc, char * argv[]) {
    using namespace rapidxml;
    xml_document<> doc;
    try {
        char doc_text[] = "<single-element/>";
        doc.parse<0>(doc_text);

        auto node = doc.first_node();
        assert(std::string("single-element") == node->name());
    } catch(parse_error & e) {
        std::cout << "Parse error: " << e.what() << std::endl << "At: " << e.where<char>() << std::endl;
    }
    try {
        char doc_text[] = "<pfx:single xmlns:pfx='urn:xmpp:example'/>";
        doc.parse<0>(doc_text);

        auto node = doc.first_node();
        std::cout << "<" << node->prefix() << ":" << node->name() << "/> " << node->xmlns() << std::endl;
        assert(std::string("single") == node->name());
    } catch(parse_error & e) {
        std::cout << "Parse error: " << e.what() << std::endl << "At: " << e.where<char>() << std::endl;
    }
    try {
        char doc_text[] = "<pfx:single xmlns:pfx='urn:xmpp:example'><pfx:firstchild/><child xmlns='urn:potato'/><pfx:child/></pfx:single>";
        doc.parse<0>(doc_text);

        auto node = doc.first_node();
        std::cout << "<" << node->prefix() << ":" << node->name() << "/> " << node->xmlns() << std::endl;
        assert(std::string("single") == node->name());
        node = node->first_node(0, "urn:potato");
        std::cout << "<" << node->prefix() << ":" << node->name() << "/> " << node->xmlns() << std::endl;
        assert(std::string("child") == node->name());
        node = node->parent()->first_node("child");
        std::cout << "<" << node->prefix() << ":" << node->name() << "/> " << node->xmlns() << std::endl;
        assert(std::string("child") == node->name());
    } catch(parse_error & e) {
        std::cout << "Parse error: " << e.what() << std::endl << "At: " << e.where<char>() << std::endl;
    }


    return 0;
}
