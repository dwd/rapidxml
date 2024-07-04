//
// Created by dave on 04/07/2024.
//

#include <gtest/gtest.h>
#include "rapidxml.hpp"
#include "rapidxml_print.hpp"

namespace {
    auto print(rapidxml::xml_document<> & doc) {
        std::string output;
        rapidxml::print(std::back_inserter(output), *doc.first_node(), rapidxml::print_no_indenting);
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
