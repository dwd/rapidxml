//
// Created by dwd on 01/07/24.
//

#include <gtest/gtest.h>

#include "rapidxml.hpp"
#include "rapidxml_print.hpp"

namespace {
    auto print(rapidxml::xml_document<> & doc) {
        std::string output;
        rapidxml::print(std::back_inserter(output), *doc.first_node());
        return output;
    }
}

TEST(Create, Node) {
    rapidxml::xml_document<> doc;
    auto node = doc.allocate_node(rapidxml::node_element, "fish", "cakes");
    doc.append_node(node);

    EXPECT_EQ(
        print(doc),
        "<fish>cakes</fish>\n"
    );
}

TEST(Create, NodeEmpty) {
    rapidxml::xml_document<> doc;
    auto node = doc.allocate_node(rapidxml::node_element, "fish");
    doc.append_node(node);

    EXPECT_EQ(
        print(doc),
        "<fish/>\n"
    );
}

TEST(Create, Node2) {
    rapidxml::xml_document<> doc;
    auto node = doc.allocate_node(rapidxml::node_element, "fish", "cakes");
    doc.append_node(node);

    EXPECT_EQ(
        print(doc),
        "<fish>cakes</fish>\n"
    );
}

static std::string s = "tuna";

std::string const & fn() {
    return s;
}

TEST(Create, NodeAttr) {
    rapidxml::xml_document<> doc;
    auto node = doc.allocate_node(rapidxml::node_element, "fish", "cakes");
    auto haddock = doc.allocate_attribute("id", "haddock");
    node->append_attribute(haddock);
    doc.append_node(node);

    EXPECT_EQ(
        print(doc),
        "<fish id=\"haddock\">cakes</fish>\n"
    );

    const std::string & s2 = fn();
    const rapidxml::xml_attribute<>::view_type & sv{s2};

    auto tuna = doc.allocate_attribute("not-id", fn());
    // These check that the same buffer is being used throughout, instead of creating temporaries.
    EXPECT_EQ(s.data(), s2.data());
    EXPECT_EQ(s.data(), sv.data());
    EXPECT_EQ(s.data(), tuna->value().data());
    node->append_attribute(tuna);
    EXPECT_EQ(haddock->next_attribute(), tuna);
    EXPECT_EQ(tuna->parent(), node);
    EXPECT_EQ(
        print(doc),
        "<fish id=\"haddock\" not-id=\"tuna\">cakes</fish>\n"
    );
    node->remove_attribute(tuna);
    EXPECT_EQ(haddock->next_attribute(), nullptr);
    EXPECT_EQ(tuna->parent(), nullptr);
    EXPECT_EQ(
        print(doc),
        "<fish id=\"haddock\">cakes</fish>\n"
    );
    node->prepend_attribute(tuna);
    EXPECT_EQ(
        print(doc),
        "<fish not-id=\"tuna\" id=\"haddock\">cakes</fish>\n"
    );
    node->value("pie");
    EXPECT_EQ(
        print(doc),
        "<fish not-id=\"tuna\" id=\"haddock\">pie</fish>\n"
    );
    node->remove_all_attributes();
    EXPECT_EQ(
        print(doc),
        "<fish>pie</fish>\n"
    );
    auto child = node->append_element({"urn:xmpp:fish:0", "shark"}, fn());
    EXPECT_EQ(s.data(), child->value().data());
    EXPECT_EQ(
            print(doc),
            "<fish>\n\t<shark xmlns=\"urn:xmpp:fish:0\">tuna</shark>\n</fish>\n"
    );
    child->append_element({"urn:xmpp:fish:0", "species"}, "tiger");
    EXPECT_EQ(
            print(doc),
            "<fish>\n\t<shark xmlns=\"urn:xmpp:fish:0\">\n\t\t<species>tiger</species>\n\t</shark>\n</fish>\n"
    );
}
