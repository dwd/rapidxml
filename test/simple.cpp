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

        std::cout << "Test three, full parse" << std::endl;
        auto node = doc.first_node();
        //doc.fixup<0>(node, true);
        //std::cout << "<" << node->prefix() << ":" << node->name() << "/> " << node->xmlns() << std::endl;
        assert(std::string("single") == node->name());
        auto child = node->first_node(0, "urn:potato");
        assert(child);
        assert(std::string("child") == child->name());
        assert(std::string("urn:potato") == child->xmlns());
        child = node->first_node();
        assert(std::string("firstchild") == child->name());
        assert(std::string("urn:xmpp:example") == child->xmlns());
        //std::cout << "<" << node->prefix() << ":" << node->name() << "/> " << node->xmlns() << std::endl;
        child = node->first_node("child");
        assert(std::string("child") == child->name());
        assert(std::string("urn:xmpp:example") == child->xmlns());
        //std::cout << "<" << node->prefix() << ":" << node->name() << "/> " << node->xmlns() << std::endl;
    } catch(parse_error & e) {
        std::cout << "Parse error: " << e.what() << std::endl << "At: " << e.where<char>() << std::endl;
    }

    try {
        char doc_text[] = "<pfx:single xmlns:pfx='urn:xmpp:example'><pfx:firstchild/><child xmlns='urn:potato'/><pfx:child/></pfx:single>";
        doc.parse<parse_fastest>(doc_text);

        std::cout << "Test three, fastest parse" << std::endl;
        auto node = doc.first_node();
        //doc.fixup<0>(node, true);
        //std::cout << "<" << node->prefix() << ":" << node->name() << "/> " << node->xmlns() << std::endl;
        assert("single" == std::string(node->name(), node->name_size()));
        assert("urn:xmpp:example" == std::string(node->xmlns(), node->xmlns_size()));
        auto child = node->first_node(0, "urn:potato");
        assert(child);
        assert("child" == std::string(child->name(), child->name_size()));
        assert("urn:potato" == std::string(child->xmlns(), child->xmlns_size()));
        child = node->first_node();
        assert("firstchild" == std::string(child->name(), child->name_size()));
        assert("urn:xmpp:example" == std::string(child->xmlns(), child->xmlns_size()));
        //std::cout << "<" << node->prefix() << ":" << node->name() << "/> " << node->xmlns() << std::endl;
        child = node->first_node("child");
        assert("child" == std::string(child->name(), child->name_size()));
        assert("urn:xmpp:example" == std::string(child->xmlns(), child->xmlns_size()));
        //std::cout << "<" << node->prefix() << ":" << node->name() << "/> " << node->xmlns() << std::endl;
    } catch(parse_error & e) {
        std::cout << "Parse error: " << e.what() << std::endl << "At: " << e.where<char>() << std::endl;
    }
    try {
        char doc_text[] = "<pfx:single xmlns:pfx='urn:xmpp:example'><pfx:firstchild/><child xmlns='urn:potato'/><pfx:child/></pfx:single>";
        doc.parse<parse_fastest>(doc_text);

        std::cout << "Test three, fixup." << std::endl;
        auto node = doc.first_node();
        doc.fixup<0>(node, true);
        //std::cout << "<" << node->prefix() << ":" << node->name() << "/> " << node->xmlns() << std::endl;
        assert("single" == std::string(node->name(), node->name_size()));
        assert("urn:xmpp:example" == std::string(node->xmlns(), node->xmlns_size()));
        auto child = node->first_node(0, "urn:potato");
        assert(child);
        assert("child" == std::string(child->name(), child->name_size()));
        assert("urn:potato" == std::string(child->xmlns(), child->xmlns_size()));
        child = node->first_node();
        assert("firstchild" == std::string(child->name(), child->name_size()));
        assert("urn:xmpp:example" == std::string(child->xmlns(), child->xmlns_size()));
        //std::cout << "<" << node->prefix() << ":" << node->name() << "/> " << node->xmlns() << std::endl;
        child = node->first_node("child");
        assert("child" == std::string(child->name(), child->name_size()));
        assert("urn:xmpp:example" == std::string(child->xmlns(), child->xmlns_size()));
        //std::cout << "<" << node->prefix() << ":" << node->name() << "/> " << node->xmlns() << std::endl;
    } catch(parse_error & e) {
        std::cout << "Parse error: " << e.what() << std::endl << "At: " << e.where<char>() << std::endl;
    }
    try {
        char doc_text[] = "<pfx:single xmlns:pfx='urn:xmpp:example'>";
        doc.parse<parse_unclosed>(doc_text);

        auto node = doc.first_node();
        std::cout << "<" << node->prefix() << ":" << node->name() << "/> " << node->xmlns() << std::endl;
        assert(std::string("single") == node->name());
    } catch(parse_error & e) {
        std::cout << "Parse error: " << e.what() << std::endl << "At: " << e.where<char>() << std::endl;
    }
    return 0;
}
