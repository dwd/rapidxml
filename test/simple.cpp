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
        doc.validate();
    } catch(parse_error & e) {
        std::cout << "Parse error: " << e.what() << std::endl << "At: " << e.where<char>() << std::endl;
    } catch(validation_error & e) {
        std::cout << "Validation error: " << e.what() << std::endl;
    }
    try {
        char doc_text[] = "<pfx:single-element/>";
        doc.parse<0>(doc_text);

        auto node = doc.first_node();
        assert(std::string("single-element") == node->name());
        std::cout << "Expecting unbound prefix (element):\n";
        doc.validate();
    } catch(parse_error & e) {
        std::cout << "Parse error: " << e.what() << std::endl << "At: " << e.where<char>() << std::endl;
    } catch(validation_error & e) {
        std::cout << "Validation error: " << e.what() << std::endl;
    }
    try {
        char doc_text[] = "<single-element attr='one' attr=\"two\"/>";
        doc.parse<0>(doc_text);

        auto node = doc.first_node();
        assert(std::string("single-element") == node->name());
        std::cout << "Expecting doubled attribute: \n";
        doc.validate();
    } catch(parse_error & e) {
        std::cout << "Parse error: " << e.what() << std::endl << "At: " << e.where<char>() << std::endl;
    } catch(validation_error & e) {
        std::cout << "Validation error: " << e.what() << std::endl;
    }
    try {
        char doc_text[] = "<single-element pfx1:attr='one' attr=\"two\"/>";
        doc.parse<0>(doc_text);

        auto node = doc.first_node();
        assert(std::string("single-element") == node->name());
        auto attr = node->first_attribute();
        assert(attr->xmlns() == 0);
        std::cout << "Expecting unbound prefix (attr): \n";
        doc.validate();
    } catch(parse_error & e) {
        std::cout << "Parse error: " << e.what() << std::endl << "At: " << e.where<char>() << std::endl;
    } catch(validation_error & e) {
        std::cout << "Validation error: " << e.what() << std::endl;
    }
    try {
        char doc_text[] = "<single-element pfx1:attr='one' pfx2:attr=\"two\" xmlns:pfx1='urn:fish' xmlns:pfx2='urn:fish'/>";
        doc.parse<0>(doc_text);

        auto node = doc.first_node();
        assert(std::string("single-element") == node->name());
        std::cout << "Expecting doubled attr (XMLNS): \n";
        doc.validate();
    } catch(parse_error & e) {
        std::cout << "Parse error: " << e.what() << std::endl << "At: " << e.where<char>() << std::endl;
    } catch(validation_error & e) {
        std::cout << "Validation error: " << e.what() << std::endl;
    }
    try {
        char doc_text[] = "<pfx:single xmlns:pfx='urn:xmpp:example'/>";
        doc.parse<0>(doc_text);

        auto node = doc.first_node();
        std::cout << "Simple: <" << node->prefix() << ":" << node->name() << "/> " << node->xmlns() << std::endl;
        assert(std::string("single") == node->name());
        doc.validate();
    } catch(parse_error & e) {
        std::cout << "Parse error: " << e.what() << std::endl << "At: " << e.where<char>() << std::endl;
    } catch(validation_error & e) {
        std::cout << "Validation error: " << e.what() << std::endl;
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
        doc.validate();
    } catch(parse_error & e) {
        std::cout << "Parse error: " << e.what() << std::endl << "At: " << e.where<char>() << std::endl;
    } catch(validation_error & e) {
        std::cout << "Validation error: " << e.what() << std::endl;
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
        doc.validate();
    } catch(parse_error & e) {
        std::cout << "Parse error: " << e.what() << std::endl << "At: " << e.where<char>() << std::endl;
    } catch(validation_error & e) {
        std::cout << "Validation error: " << e.what() << std::endl;
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
        doc.validate();
    } catch(parse_error & e) {
        std::cout << "Parse error: " << e.what() << std::endl << "At: " << e.where<char>() << std::endl;
    } catch(validation_error & e) {
        std::cout << "Validation error: " << e.what() << std::endl;
    }
    try {
        char doc_text[] = "<pfx:single xmlns:pfx='urn:xmpp:example'>";
        doc.parse<parse_open_only>(doc_text);

        auto node = doc.first_node();
        std::cout << "<" << node->prefix() << ":" << node->name() << "/> " << node->xmlns() << std::endl;
        assert(std::string("single") == node->name());
        doc.validate();
    } catch(parse_error & e) {
        std::cout << "Parse error: " << e.what() << std::endl << "At: " << e.where<char>() << std::endl;
    } catch(validation_error & e) {
        std::cout << "Validation error: " << e.what() << std::endl;
    }
    try {
        char doc_text[] = "<pfx:single xmlns='jabber:client' xmlns:pfx='urn:xmpp:example'><pfx:features><feature1/><feature2/></pfx:features><message to='me@mydomain.com' from='you@yourdomcina.com' xml:lang='en'><body>Hello!</body></message>";
        char * text = doc.parse<parse_open_only>(doc_text);

        auto node = doc.first_node();
        std::cout << "<" << node->prefix() << ":" << node->name() << "/> " << node->xmlns() << std::endl;
        assert(std::string("single") == node->name());
	std::cout << text << std::endl;
        doc.validate();
        while (*text) {
            xml_document<> subdoc;
            text = subdoc.parse<parse_parse_one>(text, doc);
            auto node = subdoc.first_node();
            auto xmlns = node->xmlns();
            std::cout << "<" << node->name() << " xmlns='" << node->xmlns() << "'/>" << std::endl;
            subdoc.validate();
        }
    } catch(parse_error & e) {
        std::cout << "Parse error: " << e.what() << std::endl << "At: " << e.where<char>() << std::endl;
    } catch(validation_error & e) {
        std::cout << "Validation error: " << e.what() << std::endl;
    }
    try {
        char doc_text[] = "<pfx:single xmlns:pfx='urn:xmpp:example'><pfx:features><feature1/><feature2/></p";
        char * text = doc.parse<parse_open_only|parse_fastest>(doc_text);

        auto node = doc.first_node();
        std::cout << "<" << node->prefix() << ":" << node->name() << "/> " << node->xmlns() << std::endl;
        assert(std::string("single") == std::string(node->name(), node->name_size()));
	std::cout << text << std::endl;
        doc.validate();
        while (*text) {
            xml_document<> subdoc;
            text = subdoc.parse<parse_parse_one>(text, doc);
            subdoc.validate();
            auto node = doc.first_node();
            std::cout << "<" << node->name() << " xmlns='" << node->xmlns() << "'/>" << std::endl;
        }
    } catch(parse_error & e) {
        std::cout << "Parse error: " << e.what() << std::endl << "At: " << e.where<char>() << std::endl;
    } catch(validation_error & e) {
        std::cout << "Validation error: " << e.what() << std::endl;
    }
    return 0;
}
