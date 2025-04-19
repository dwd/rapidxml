//
// Created by dave on 10/07/2024.
//

#include <gtest/gtest.h>
#include <list>
#include <algorithm>
#include <ranges>
#include "rapidxml.hpp"

TEST(Iterators, Nodes) {
    std::string xml = "<children><one/><two/><three/></children>";
    rapidxml::xml_document<> doc;
    doc.parse<rapidxml::parse_full>(xml);
    int i = 0;
    for (auto & child : doc.first_node()->children()) {
        ++i;
        switch(i) {
            case 1:
                EXPECT_EQ(child.name(), "one");
                break;
            case 2:
                EXPECT_EQ(child.name(), "two");
                break;
            case 3:
                EXPECT_EQ(child.name(), "three");
                break;
        }
    }
    EXPECT_EQ(i, 3);
}

TEST(Iterators, Attributes) {
    std::string xml = R"(<children one="1" two="2" three="3"/>)";
    rapidxml::xml_document<> doc;
    doc.parse<rapidxml::parse_full>(xml);
    int i = 0;
    for (auto & child : doc.first_node()->attributes()) {
        ++i;
        switch(i) {
            case 1:
                EXPECT_EQ(child.name(), "one");
                break;
            case 2:
                EXPECT_EQ(child.name(), "two");
                break;
            case 3:
                EXPECT_EQ(child.name(), "three");
                break;
        }
    }
    EXPECT_EQ(i, 3);
}

TEST(Predicates, Nodes) {
    std::string xml = "<children><one/><two/><three/></children>";
    rapidxml::xml_document<> doc;
    doc.parse<rapidxml::parse_full>(xml);
    auto r = doc.first_node()->children();
    for (auto const & child : r | std::ranges::views::filter([](auto const & n) { return n.name() == "two"; })) {
        EXPECT_EQ(child.name(), "two");
    }
    auto c = std::ranges::count_if(r, [](auto const & n) { return n.name() == "two"; });
    EXPECT_EQ(c, 1);
    auto match = std::ranges::find_if(r, [](auto const & n) { return n.name() == "two"; });
    EXPECT_EQ(match->name(), "two");
}

TEST(Predicates, AllNodes) {
    std::string xml = "<children><one><two/></one><three><four><five/></four><six/></three></children>";
    rapidxml::xml_document<> doc;
    doc.parse<rapidxml::parse_full>(xml);
    auto it = rapidxml::descendant_iterator<>(doc.first_node());
    EXPECT_EQ(it->name(), "one");
    ++it;
    EXPECT_EQ(it->name(), "two");
    ++it;
    EXPECT_EQ(it->name(), "three");
    ++it;
    EXPECT_EQ(it->name(), "four");
    ++it;
    EXPECT_EQ(it->name(), "five");
    ++it;
    EXPECT_EQ(it->name(), "six");
    ++it;
    EXPECT_FALSE(it.valid());
}

TEST(Predicates, AllNodesRev) {
    std::string xml = "<children><one><two/></one><three><four><five/></four><six/></three></children>";
    rapidxml::xml_document<> doc;
    doc.parse<rapidxml::parse_full>(xml);
    auto it = rapidxml::descendant_iterator<>(doc.first_node());
    EXPECT_EQ(it->name(), "one");
    ++it;
    EXPECT_EQ(it->name(), "two");
    ++it;
    EXPECT_EQ(it->name(), "three");
    ++it;
    EXPECT_EQ(it->name(), "four");
    ++it;
    EXPECT_EQ(it->name(), "five");
    ++it;
    EXPECT_EQ(it->name(), "six");
    --it;
    EXPECT_EQ(it->name(), "five");
    --it;
    EXPECT_EQ(it->name(), "four");
    --it;
    EXPECT_EQ(it->name(), "three");
    --it;
    EXPECT_EQ(it->name(), "two");
    --it;
    EXPECT_EQ(it->name(), "one");
}

TEST(Predicates, Attributes) {
    std::string xml = R"(<children one="1" two="2" three="3"/>)";
    rapidxml::xml_document<> doc;
    doc.parse<rapidxml::parse_full>(xml);
    auto r = doc.first_node()->attributes();
    for (auto const & child : r | std::ranges::views::filter([](auto const & n) { return n.name() == "two"; })) {
        EXPECT_EQ(child.name(), "two");
    }
    auto c = std::ranges::count_if(r, [](auto const & n) { return n.name() == "two"; });
    EXPECT_EQ(c, 1);
    auto match = std::ranges::find_if(r, [](auto const & n) { return n.name() == "two"; });
    EXPECT_EQ(match->name(), "two");
    auto match2 = std::ranges::find_if(doc.first_node()->attributes(), [](auto const & n) { return n.name() == "two"; });
    EXPECT_EQ(match2->name(), "two");
}
