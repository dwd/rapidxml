//
// Created by dave on 04/07/2024.
//

#include <gtest/gtest.h>
#include "rapidxml.hpp"
#include "rapidxml_print.hpp"

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
    auto check2 = doc.first_node()->append_element(name, "another'");
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
}

TEST(RoundTrip, SimpleApos) {
    const char input[] = "<simple arg=\"'\"/>";
    std::vector<char> buffer{input, input + sizeof(input)};
    rapidxml::xml_document<> doc;
    doc.parse<rapidxml::parse_full>(buffer.data());
    auto output = print(doc);
    // Have we parsed correctly?
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
    EXPECT_EQ(expected, output);
    // Have we mutated the underlying buffer?
    EXPECT_EQ(input, std::string(buffer.data(), buffer.size() - 1));
}

TEST(RoundTrip, Everything) {
    const char input[] = "<?xml charset='utf-8' ?><!DOCTYPE ><simple arg=\"&apos;\"><!-- Comment here --></simple>";
    const char expected[] = "<?xml charset=\"utf-8\"?><!DOCTYPE ><simple arg=\"'\"><!-- Comment here --></simple>";
    std::vector<char> buffer{input, input + sizeof(input)};
    rapidxml::xml_document<> doc;
    doc.parse<rapidxml::parse_full>(buffer.data());
    auto output = print(doc);
    // Have we parsed correctly?
    EXPECT_EQ(expected, output);
    // Have we mutated the underlying buffer?
    EXPECT_EQ(input, std::string(buffer.data(), buffer.size() - 1));
}
