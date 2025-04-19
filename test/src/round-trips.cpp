//
// Created by dave on 04/07/2024.
//

#include <gtest/gtest.h>
#include "rapidxml.hpp"
#include "rapidxml_print.hpp"
#include "rapidxml_iterators.hpp"

namespace {
    auto print(rapidxml::xml_document<> & doc) {
        std::string output;
        rapidxml::print(std::back_inserter(output), doc, rapidxml::print_no_indenting);
        return output;
    }
}

TEST(RoundTrip, Simple) {
    const char input[] = "<simple/>";
    std::vector<char> buffer{input, input + sizeof(input)};
    rapidxml::xml_document<> doc;
    doc.parse<rapidxml::parse_full>(buffer.data());
    auto output = print(doc);
    // Have we parsed correctly?
    EXPECT_EQ(input, output);
    // Have we mutated the underlying buffer?
    EXPECT_EQ(input, std::string(buffer.data(), buffer.size() - 1));
}

TEST(RoundTrip, SimpleMod) {
    const char input[] = "<pfx:simple xmlns:pfx=\"this\"><pfx:that/></pfx:simple>";
    std::vector<char> buffer{input, input + sizeof(input)};
    rapidxml::xml_document<> doc;
    doc.parse<rapidxml::parse_full>(buffer.data());
    auto output = print(doc);
    // Have we parsed correctly?
    EXPECT_EQ(input, output);
    // Have we mutated the underlying buffer?
    EXPECT_EQ(input, std::string(buffer.data(), buffer.size() - 1));
    auto that = doc.first_node()->first_node();
    doc.first_node()->remove_node(that);
    auto output2 = print(doc);
    EXPECT_EQ(output2, "<pfx:simple xmlns:pfx=\"this\"/>");
    std::string xmlns = "that";
    std::string name = "this";
    auto check = doc.first_node()->append_element(name, "the other");
    doc.first_node()->append_element(name, "another'");
    EXPECT_EQ(name, "this");
    EXPECT_EQ(check->name(), name);
    EXPECT_EQ(check->name().data(), name.data());
    doc.first_node()->append_element("odd", "the other");
    doc.first_node()->append_element({xmlns, name}, "the other");
    EXPECT_EQ(name, "this");
    EXPECT_EQ(check->name(), name);
    EXPECT_EQ(check->name().data(), name.data());
    doc.first_node()->append_element({"this", "that"}, "the other");
    doc.first_node()->append_element(name, "last time");
    EXPECT_EQ(name, "this");
    EXPECT_EQ(check->name(), name);
    EXPECT_EQ(check->name().data(), name.data());
    auto output3 = print(doc);
    EXPECT_EQ(name, "this");
    EXPECT_EQ(check->name(), name);
    EXPECT_EQ(check->name().data(), name.data());
    EXPECT_EQ(output3, "<pfx:simple xmlns:pfx=\"this\"><this>the other</this><this>another&apos;</this><odd>the other</odd><this xmlns=\"that\">the other</this><pfx:that>the other</pfx:that><this>last time</this></pfx:simple>");
    rapidxml::xml_document<> doc2;
    doc2.clone_node(doc.first_node(), true);
    auto output4 = print(doc);
    EXPECT_EQ(output3, output4);
}

TEST(RoundTrip, SimpleApos) {
    const char input[] = "<simple arg=\"'\"/>";
    std::vector<char> buffer{input, input + sizeof(input)};
    rapidxml::xml_document<> doc;
    doc.parse<rapidxml::parse_full>(buffer.data());
    auto output = print(doc);
    // Have we parsed correctly?
    rapidxml::xml_document<> doc2;
    for (auto & child : rapidxml::children(doc)) {
        doc2.append_node(doc2.clone_node(&child, true));
    }
    EXPECT_EQ(input, print(doc2));
    EXPECT_EQ(input, output);
    // Have we mutated the underlying buffer?
    EXPECT_EQ(input, std::string(buffer.data(), buffer.size() - 1));
}

TEST(RoundTrip, SimpleApos2) {
    const char input[] = "<simple arg=\"&apos;\"/>";
    const char expected[] = "<simple arg=\"'\"/>";
    std::vector<char> buffer{input, input + sizeof(input)};
    rapidxml::xml_document<> doc;
    doc.parse<rapidxml::parse_full>(buffer.data());
    auto output = print(doc);
    EXPECT_EQ(doc.first_node()->first_attribute()->value(), "'");
    // Have we parsed correctly?
    rapidxml::xml_document<> doc2;
    for (auto & child : rapidxml::children(doc)) {
        doc2.append_node(doc2.clone_node(&child, true));
    }
    EXPECT_EQ(expected, print(doc2));
    EXPECT_EQ(expected, output);
    // Have we mutated the underlying buffer?
    EXPECT_EQ(input, std::string(buffer.data(), buffer.size() - 1));
}

TEST(RoundTrip, SimpleLtBody) {
    const char input[] = "<simple arg=\"&apos;\">&lt;</simple>";
    const char expected[] = "<simple arg=\"'\">&lt;</simple>";
    std::vector<char> buffer{input, input + sizeof(input)};
    rapidxml::xml_document<> doc;
    doc.parse<rapidxml::parse_full>(buffer.data());
    auto output = print(doc);
    EXPECT_EQ(doc.first_node()->value(), "<");
    EXPECT_EQ(doc.first_node()->first_attribute()->value(), "'");
    // Have we parsed correctly?
    rapidxml::xml_document<> doc2;
    for (auto & child : rapidxml::children(doc)) {
        doc2.append_node(doc2.clone_node(&child, true));
    }
    EXPECT_EQ(expected, print(doc2));
    EXPECT_EQ(expected, output);
    // Have we mutated the underlying buffer?
    EXPECT_EQ(input, std::string(buffer.data(), buffer.size() - 1));
}

TEST(RoundTrip, MutateBody) {
    const char input[] = "<simple arg=\"&apos;\">&lt;</simple>";
    const char expected[] = "<simple arg=\"'\">&lt;</simple>";
    const char expected2[] = "<simple arg=\"'\">new value</simple>";
    std::vector<char> buffer{input, input + sizeof(input)};
    rapidxml::xml_document<> doc;
    doc.parse<rapidxml::parse_full>(buffer.data());
    auto output = print(doc);
    EXPECT_EQ(expected, output);
    // Have we mutated the underlying buffer?
    EXPECT_EQ(input, std::string(buffer.data(), buffer.size() - 1));
    doc.first_node()->value("new value");
    EXPECT_EQ(doc.first_node()->value_raw(), "");
    EXPECT_EQ(doc.first_node()->value(), "new value");
    EXPECT_EQ(expected2, print(doc));
}

TEST(RoundTrip, Everything) {
    const char input[] = "<?xml charset='utf-8' ?><!DOCTYPE ><simple arg=\"&apos;\"><!-- Comment here --></simple>";
    const char expected[] = "<?xml charset=\"utf-8\"?><!DOCTYPE ><simple arg=\"'\"><!-- Comment here --></simple>";
    std::vector<char> buffer{input, input + sizeof(input)};
    rapidxml::xml_document<> doc;
    doc.parse<rapidxml::parse_full>(buffer.data());
    auto output = print(doc);
    rapidxml::xml_document<> doc2;
    for (auto & child : rapidxml::children(doc)) {
        doc2.append_node(doc2.clone_node(&child, true));
    }
    EXPECT_EQ(expected, print(doc2));
    // Have we parsed correctly?
    EXPECT_EQ(expected, output);
    // Have we mutated the underlying buffer?
    EXPECT_EQ(input, std::string(buffer.data(), buffer.size() - 1));
}

TEST(RoundTrip, EverythingStream) {
    const char input[] = "<?xml charset='utf-8' ?><!DOCTYPE ><simple arg=\"&apos;\"><!-- Comment here --></simple>";
    const char expected[] = "<?xml charset=\"utf-8\"?>\n<!DOCTYPE >\n<simple arg=\"'\">\n\t<!-- Comment here -->\n</simple>\n\n";
    std::vector<char> buffer{input, input + sizeof(input) - 1};
    rapidxml::xml_document<> doc;
    doc.parse<rapidxml::parse_full>(buffer);
    std::stringstream ss1;
    ss1 << doc;
    auto output = ss1.str();
    rapidxml::xml_document<> doc2;
    for (auto & child : doc.children()) {
        doc2.append_node(doc2.clone_node(&child, true));
    }
    std::stringstream ss2;
    ss2 << doc2;
    EXPECT_EQ(expected, ss2.str());
    // Have we parsed correctly?
    EXPECT_EQ(expected, output);
    // Have we mutated the underlying buffer?
    EXPECT_EQ(input, std::string(buffer.data(), buffer.size()));
}
