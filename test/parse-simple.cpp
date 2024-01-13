//
// Created by dwd on 1/13/24.
//

#include <gtest/gtest.h>
#include <rapidxml.hpp>

TEST(Parser, SingleElement) {
    char doc_text[] = "<single-element/>";
    rapidxml::xml_document<> doc;
    doc.parse<0>(doc_text);

    auto node = doc.first_node();
    EXPECT_NE(nullptr, node);
    EXPECT_NE(nullptr, node->name());
    EXPECT_STREQ("single-element", node->name());
    doc.validate();
}

TEST(Parser, UnboundPrefix) {
    rapidxml::xml_document<> doc;
    char doc_text[] = "<pfx:single-element/>";
    doc.parse<0>(doc_text);

    auto node = doc.first_node();
    EXPECT_STREQ("single-element", node->name());
    EXPECT_THROW(
        doc.validate(),
        rapidxml::element_xmlns_unbound
        );
}

TEST(Parser, DuplicateAttribute) {
    rapidxml::xml_document<> doc;
    char doc_text[] = "<single-element attr='one' attr=\"two\"/>";
    doc.parse<0>(doc_text);

    auto node = doc.first_node();
    assert(std::string("single-element") == node->name());
    EXPECT_THROW(
        doc.validate(),
        rapidxml::duplicate_attribute
        );
}

TEST(Parser, UnboundAttrPrefix) {
    rapidxml::xml_document<> doc;
    char doc_text[] = "<single-element pfx1:attr='one' attr=\"two\"/>";
    doc.parse<0>(doc_text);

    auto node = doc.first_node();
    assert(std::string("single-element") == node->name());
    auto attr = node->first_attribute();
    assert(attr->xmlns() == 0);
    EXPECT_THROW(
        doc.validate(),
        rapidxml::attr_xmlns_unbound
        );
}


TEST(Parser, DuplicateAttrPrefix) {
    rapidxml::xml_document<> doc;
    char doc_text[] = "<single-element pfx1:attr='one' pfx2:attr=\"two\" xmlns:pfx1='urn:fish' xmlns:pfx2='urn:fish'/>";
    doc.parse<0>(doc_text);

    auto node = doc.first_node();
    assert(std::string("single-element") == node->name());
    EXPECT_THROW(
        doc.validate(),
        rapidxml::duplicate_attribute
        );
}


TEST(Parser, Xmlns) {
    rapidxml::xml_document<> doc;
    char doc_text[] = "<pfx:single xmlns:pfx='urn:xmpp:example'/>";
    doc.parse<0>(doc_text);

    auto node = doc.first_node();
    EXPECT_STREQ("single", node->name());
    EXPECT_STREQ("pfx", node->prefix());
    EXPECT_STREQ("urn:xmpp:example", node->xmlns());
    doc.validate();
}

TEST(Parser, ChildXmlns) {
    rapidxml::xml_document<> doc;
    char doc_text[] = "<pfx:single xmlns:pfx='urn:xmpp:example'><pfx:firstchild/><child xmlns='urn:potato'/><pfx:child/></pfx:single>";
    doc.parse<0>(doc_text);

    auto node = doc.first_node();
    EXPECT_STREQ("single", node->name());
    auto child = node->first_node(0, "urn:potato");
    ASSERT_NE(nullptr, child);
    EXPECT_STREQ("child", child->name());
    EXPECT_STREQ("urn:potato", child->xmlns());
    child = node->first_node();
    EXPECT_STREQ("firstchild", child->name());
    EXPECT_STREQ("urn:xmpp:example", child->xmlns());
    child = node->first_node("child");
    EXPECT_STREQ("child", child->name());
    EXPECT_STREQ("urn:xmpp:example", child->xmlns());
    doc.validate();
}

TEST(ParseOptions, Fastest) {
    rapidxml::xml_document<> doc;
    char doc_text[] = "<pfx:single xmlns:pfx='urn:xmpp:example'><pfx:firstchild/><child xmlns='urn:potato'/><pfx:child/></pfx:single>";
    doc.parse<rapidxml::parse_fastest>(doc_text);

    auto node = doc.first_node();
    EXPECT_EQ("single", std::string(node->name(), node->name_size()));
    EXPECT_EQ("single", node->name_str());
    EXPECT_EQ("urn:xmpp:example", std::string(node->xmlns(), node->xmlns_size()));
    auto child = node->first_node(0, "urn:potato");
    ASSERT_NE(nullptr, child);
    EXPECT_EQ("child", std::string(child->name(), child->name_size()));
    EXPECT_EQ("urn:potato", std::string(child->xmlns(), child->xmlns_size()));
    child = node->first_node();
    EXPECT_EQ("firstchild", std::string(child->name(), child->name_size()));
    EXPECT_EQ("urn:xmpp:example", std::string(child->xmlns(), child->xmlns_size()));
    child = node->first_node("child");
    EXPECT_EQ("child", std::string(child->name(), child->name_size()));
    EXPECT_EQ("urn:xmpp:example", std::string(child->xmlns(), child->xmlns_size()));
    doc.validate();
}

TEST(ParseOptions, Fixup) {
    rapidxml::xml_document<> doc;
    char doc_text[] = "<pfx:single xmlns:pfx='urn:xmpp:example'><pfx:firstchild/><child xmlns='urn:potato'/><pfx:child/></pfx:single>";
    doc.parse<rapidxml::parse_fastest>(doc_text);

    auto node = doc.first_node();
    doc.fixup<0>(node, true);
    EXPECT_EQ("single", std::string(node->name(), node->name_size()));
    EXPECT_EQ("urn:xmpp:example", std::string(node->xmlns(), node->xmlns_size()));
    auto child = node->first_node(0, "urn:potato");
    ASSERT_NE(nullptr, child);
    EXPECT_EQ("child", std::string(child->name(), child->name_size()));
    EXPECT_EQ("urn:potato", std::string(child->xmlns(), child->xmlns_size()));
    child = node->first_node();
    EXPECT_EQ("firstchild", std::string(child->name(), child->name_size()));
    EXPECT_EQ("urn:xmpp:example", std::string(child->xmlns(), child->xmlns_size()));
    child = node->first_node("child");
    EXPECT_EQ("child", std::string(child->name(), child->name_size()));
    EXPECT_EQ("urn:xmpp:example", std::string(child->xmlns(), child->xmlns_size()));
    doc.validate();
}

TEST(ParseOptions, OpenOnly) {
    rapidxml::xml_document<> doc;
    char doc_text[] = "<pfx:single xmlns:pfx='urn:xmpp:example'>";
    doc.parse<rapidxml::parse_open_only>(doc_text);

    auto node = doc.first_node();
    EXPECT_STREQ("single", node->name());
    EXPECT_STREQ("pfx", node->prefix());
    EXPECT_STREQ("urn:xmpp:example", node->xmlns());
    doc.validate();
}

TEST(ParseOptions, ParseOne) {
    rapidxml::xml_document<> doc;
    char doc_text[] = "<pfx:single xmlns='jabber:client' xmlns:pfx='urn:xmpp:example'><pfx:features><feature1/><feature2/></pfx:features><message to='me@mydomain.com' from='you@yourdomcina.com' xml:lang='en'><body>Hello!</body></message>";
    char * text = doc.parse<rapidxml::parse_open_only>(doc_text);

    auto node = doc.first_node();
    EXPECT_STREQ("single", node->name());
    EXPECT_STREQ("pfx", node->prefix());
    EXPECT_STREQ("urn:xmpp:example", node->xmlns());
    EXPECT_STREQ("<pfx:features><feature1/><feature2/></pfx:features><message to='me@mydomain.com' from='you@yourdomcina.com' xml:lang='en'><body>Hello!</body></message>", text);
    doc.validate();
    unsigned counter = 0;
    while (*text) {
        rapidxml::xml_document<> subdoc;
        text = subdoc.parse<rapidxml::parse_parse_one>(text, doc);
        auto node = subdoc.first_node();
        ASSERT_NE(nullptr, node);
        switch(++counter) {
        case 1:
            EXPECT_STREQ("features", node->name());
            EXPECT_STREQ("urn:xmpp:example", node->xmlns());
            break;
        case 2:
            EXPECT_STREQ("message", node->name());
            EXPECT_STREQ("jabber:client", node->xmlns());
            break;
        default:
            FAIL();
        }
        subdoc.validate();
    }
}

TEST(ParseOptions, OpenOnlyFastest) {
    rapidxml::xml_document<> doc;
    char doc_text[] = "<pfx:single xmlns='jabber:client' xmlns:pfx='urn:xmpp:example'><pfx:features><feature1/><feature2/></pfx:features><message to='me@mydomain.com' from='you@yourdomcina.com' xml:lang='en'><body>Hello!</body></message>";
    char * text = doc.parse<rapidxml::parse_open_only|rapidxml::parse_fastest>(doc_text);

    auto node = doc.first_node();
    EXPECT_EQ("single", node->name_str());
    EXPECT_EQ("pfx", std::string(node->prefix(), node->prefix_size()));
    EXPECT_EQ("urn:xmpp:example", node->xmlns_str());
    EXPECT_STREQ("<pfx:features><feature1/><feature2/></pfx:features><message to='me@mydomain.com' from='you@yourdomcina.com' xml:lang='en'><body>Hello!</body></message>", text);
    doc.validate();
    unsigned counter = 0;
    while (*text) {
        rapidxml::xml_document<> subdoc;
        text = subdoc.parse<rapidxml::parse_parse_one>(text, doc);
        auto node = subdoc.first_node();
        ASSERT_NE(nullptr, node);
        switch(++counter) {
        case 1:
            EXPECT_EQ("features", node->name_str());
            EXPECT_EQ("urn:xmpp:example", node->xmlns_str());
            break;
        case 2:
            EXPECT_EQ("message", node->name_str());
            EXPECT_EQ("jabber:client", node->xmlns_str());
            break;
        default:
            FAIL();
        }
        subdoc.validate();
    }
}

