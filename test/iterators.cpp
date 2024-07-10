//
// Created by dave on 10/07/2024.
//

#include <gtest/gtest.h>
#include "rapidxml.hpp"
#include "rapidxml_iterators.hpp"

TEST(Iterators, Nodes) {
    std::string xml = "<children><one/><two/><three/></children>";
    rapidxml::xml_document<> doc;
    doc.parse<rapidxml::parse_full>(xml);
    int i = 0;
    for (auto & child : rapidxml::children(doc.first_node())) {
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
    for (auto & child : rapidxml::attributes(doc.first_node())) {
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
