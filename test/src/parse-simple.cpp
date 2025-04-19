//
// Created by dwd on 1/13/24.
//

#include <gtest/gtest.h>
#include <flxml.hpp>

TEST(Parser, SingleElement) {
    char doc_text[] = "<single-element/>";
    flxml::xml_document<> doc;
    doc.parse<0>(doc_text);

    auto node = doc.first_node();
    EXPECT_NE(nullptr, node);
    EXPECT_FALSE(node->name().empty());
    EXPECT_EQ("single-element", node->name());
    doc.validate();
}

TEST(Parser, DefaultElementNS) {
    char doc_text[] = "<element xmlns='this'><child/></element>";
    flxml::xml_document<> doc;
    doc.parse<flxml::parse_fastest | flxml::parse_parse_one>(doc_text);

    auto node = doc.first_node();
    EXPECT_NE(nullptr, node);
    EXPECT_FALSE(node->name().empty());
    EXPECT_EQ("element", node->name());
    EXPECT_EQ(node->xmlns(), "this");
    auto child = node->first_node();
    EXPECT_EQ(child->name(), "child");
    EXPECT_EQ(child->xmlns(), "this");
    doc.validate();
    auto no_node = child->next_sibling();
    EXPECT_THROW(no_node->xmlns(), flxml::no_such_node);
}

TEST(Parser, UnboundPrefix) {
    flxml::xml_document<> doc;
    char doc_text[] = "<pfx:single-element/>";
    doc.parse<0>(doc_text);

    auto node = doc.first_node();
    EXPECT_EQ("single-element", node->name());
    EXPECT_THROW(
        doc.validate(),
        flxml::element_xmlns_unbound
        );
}

TEST(Parser, DuplicateAttribute) {
    flxml::xml_document<> doc;
    char doc_text[] = "<single-element attr='one' attr=\"two\"/>";
    doc.parse<0>(doc_text);

    auto node = doc.first_node();
    EXPECT_EQ("single-element", node->name());
    EXPECT_THROW(
        doc.validate(),
        flxml::duplicate_attribute
        );
}

TEST(Parser, UnboundAttrPrefix) {
    flxml::xml_document<> doc;
    char doc_text[] = "<single-element pfx1:attr='one' attr=\"two\"/>";
    doc.parse<0>(doc_text);

    auto node = doc.first_node();
    EXPECT_EQ("single-element", node->name());
    auto attr = node->first_attribute();
    EXPECT_THROW(
        doc.validate(),
        flxml::attr_xmlns_unbound
        );
    EXPECT_THROW(
        attr->xmlns(),
        flxml::attr_xmlns_unbound
    );
}


TEST(Parser, DuplicateAttrPrefix) {
    flxml::xml_document<> doc;
    char doc_text[] = "<single-element pfx1:attr='one' pfx2:attr=\"two\" xmlns:pfx1='urn:fish' xmlns:pfx2='urn:fish'/>";
    doc.parse<0>(doc_text);

    auto node = doc.first_node();
    assert(std::string("single-element") == node->name());
    EXPECT_THROW(
        doc.validate(),
        flxml::duplicate_attribute
    );
}


TEST(Parser, Xmlns) {
    flxml::xml_document<> doc;
    char doc_text[] = "<pfx:single xmlns:pfx='urn:xmpp:example'/>";
    doc.parse<0>(doc_text);

    auto node = doc.first_node();
    EXPECT_EQ("single", node->name());
    EXPECT_EQ("pfx", node->prefix());
    EXPECT_EQ("urn:xmpp:example", node->xmlns());
    doc.validate();
}

TEST(Parser, ChildXmlns) {
    flxml::xml_document<> doc;
    char doc_text[] = "<pfx:single xmlns:pfx='urn:xmpp:example' foo='bar'><pfx:firstchild/><child xmlns='urn:potato'/><pfx:child/></pfx:single>";
    doc.parse<0>(doc_text);

    auto node = doc.first_node();
    EXPECT_EQ("single", node->name());
    auto child = node->first_node({}, "urn:potato");
    ASSERT_NE(nullptr, child);
    EXPECT_EQ("child", child->name());
    EXPECT_EQ("urn:potato", child->xmlns());
    child = node->first_node();
    EXPECT_EQ("firstchild", child->name());
    EXPECT_EQ("urn:xmpp:example", child->xmlns());
    child = child->next_sibling();
    EXPECT_EQ("child", child->name());
    EXPECT_EQ("urn:potato", child->xmlns());
    child = child->next_sibling();
    EXPECT_EQ("child", child->name());
    EXPECT_EQ("urn:xmpp:example", child->xmlns());
    child = node->first_node("child");
    EXPECT_EQ("child", child->name());
    EXPECT_EQ("urn:xmpp:example", child->xmlns());
    child = node->first_node()->next_sibling({}, "urn:xmpp:example");
    EXPECT_EQ("child", child->name());
    EXPECT_EQ("urn:xmpp:example", child->xmlns());
    child = node->first_node()->next_sibling("child");
    // This will default to the same namespace as the first child ndoe.
    EXPECT_EQ("child", child->name());
    EXPECT_EQ("urn:xmpp:example", child->xmlns());
    auto attr = node->first_attribute();
    EXPECT_EQ(attr->xmlns(), "http://www.w3.org/2000/xmlns/");
    EXPECT_EQ(attr->local_name(), "pfx");
    EXPECT_EQ(attr->name(), "xmlns:pfx");
    EXPECT_EQ(attr->value(), "urn:xmpp:example");
    attr = attr->next_attribute();
    EXPECT_EQ(attr->xmlns(), "");
    EXPECT_EQ(attr->local_name(), "foo");
    EXPECT_EQ(attr->name(), "foo");
    EXPECT_EQ(attr->value(), "bar");
    doc.validate();
}

TEST(Parser, HandleEOF){
    flxml::xml_document<> doc;
    char doc_text[] = "<open_element>";
    EXPECT_THROW(
        doc.parse<0>(doc_text),
        flxml::eof_error
    );
}

TEST(ParseOptions, Fastest) {
    flxml::xml_document<> doc;
    char doc_text[] = "<pfx:single xmlns:pfx='urn:xmpp:example'><pfx:firstchild/><child xmlns='urn:potato'/><pfx:child/></pfx:single>";
    doc.parse<flxml::parse_fastest>(doc_text);

    auto node = doc.first_node();
    EXPECT_EQ("single", node->name());
    EXPECT_EQ("urn:xmpp:example", node->xmlns());
    auto child = node->first_node({}, "urn:potato");
    ASSERT_NE(nullptr, child);
    EXPECT_EQ("child", child->name());
    EXPECT_EQ("urn:potato", child->xmlns());
    child = node->first_node();
    EXPECT_EQ("firstchild", child->name());
    EXPECT_EQ("urn:xmpp:example", child->xmlns());
    child = node->first_node("child");
    EXPECT_EQ("child", child->name());
    EXPECT_EQ("urn:xmpp:example", child->xmlns());
    doc.validate();
}

TEST(ParseOptions, OpenOnly) {
    flxml::xml_document<> doc;
    char doc_text[] = "<pfx:single xmlns:pfx='urn:xmpp:example'>";
    doc.parse<flxml::parse_open_only>(doc_text);

    auto node = doc.first_node();
    EXPECT_EQ("single", node->name());
    EXPECT_EQ("pfx", node->prefix());
    EXPECT_EQ("urn:xmpp:example", node->xmlns());
    doc.validate();
}

TEST(ParseOptions, ParseOne) {
    flxml::xml_document<> doc;
    char doc_text[] = "<pfx:single xmlns='jabber:client' xmlns:pfx='urn:xmpp:example'><pfx:features><feature1/><feature2/></pfx:features><message to='me@mydomain.com' from='you@yourdomcina.com' xml:lang='en'><body>Hello!</body></message>";
    const char * text = doc.parse<flxml::parse_open_only>(doc_text);

    {
        auto node = doc.first_node();
        EXPECT_EQ("single", node->name());
        EXPECT_EQ("pfx", node->prefix());
        EXPECT_EQ("urn:xmpp:example", node->xmlns());
        EXPECT_STREQ(
                "<pfx:features><feature1/><feature2/></pfx:features><message to='me@mydomain.com' from='you@yourdomcina.com' xml:lang='en'><body>Hello!</body></message>",
                text);
    }
    doc.validate();
    unsigned counter = 0;
    while (*text) {
        flxml::xml_document<> subdoc;
        text = subdoc.parse<flxml::parse_parse_one>(text, &doc);
        auto node = subdoc.first_node();
        ASSERT_NE(nullptr, node);
        switch(++counter) {
        case 1:
            EXPECT_EQ("features", node->name());
            EXPECT_EQ("urn:xmpp:example", node->xmlns());
            break;
        case 2:
            EXPECT_EQ("message", node->name());
            EXPECT_EQ("jabber:client", node->xmlns());
            break;
        default:
            FAIL();
        }
        subdoc.validate();
    }
}

TEST(ParseOptions, OpenOnlyFastest) {
    flxml::xml_document<> doc;
    char doc_text[] = "<pfx:single xmlns='jabber:client' xmlns:pfx='urn:xmpp:example'><pfx:features><feature1/><feature2/></pfx:features><message to='me@mydomain.com' from='you@yourdomcina.com' xml:lang='en'><body>Hello!</body></message>";
    const char * text = doc.parse<flxml::parse_open_only|flxml::parse_fastest>(doc_text);

    {
        auto node = doc.first_node();
        EXPECT_EQ("single", node->name());
        EXPECT_EQ("pfx", node->prefix());
        EXPECT_EQ("urn:xmpp:example", node->xmlns());
        EXPECT_STREQ(
                "<pfx:features><feature1/><feature2/></pfx:features><message to='me@mydomain.com' from='you@yourdomcina.com' xml:lang='en'><body>Hello!</body></message>",
                text);
    }
    doc.validate();
    unsigned counter = 0;
    while (*text) {
        flxml::xml_document<> subdoc;
        text = subdoc.parse<flxml::parse_parse_one>(text, &doc);
        auto node = subdoc.first_node();
        ASSERT_NE(nullptr, node);
        switch(++counter) {
        case 1:
            EXPECT_EQ("features", node->name());
            EXPECT_EQ("urn:xmpp:example", node->xmlns());
            break;
        case 2:
            EXPECT_EQ("message", node->name());
            EXPECT_EQ("jabber:client", node->xmlns());
            break;
        default:
            FAIL();
        }
        subdoc.validate();
    }
}

TEST(Parser_Emoji, Single) {
    std::string foo{"<h>&apos;</h>"};
    flxml::xml_document<> doc;
    doc.parse<flxml::parse_default>(foo);
    EXPECT_EQ("'", doc.first_node()->value());
}

TEST(Parser_Emoji, SingleUni) {
    std::string foo{"<h>&#1234;</h>"};
    flxml::xml_document<> doc;
    doc.parse<flxml::parse_default>(foo);
    EXPECT_EQ("\xD3\x92", doc.first_node()->value());
}

TEST(Parser_Emoji, SingleEmoji) {
    std::string foo{"<h>&#128512;</h>"};
    flxml::xml_document<> doc;
    doc.parse<flxml::parse_default>(foo);
    EXPECT_EQ("\xF0\x9F\x98\x80", doc.first_node()->value());
    EXPECT_EQ(4, doc.first_node()->value().size());
}

TEST(Parser_Emoji, SingleEmojiReuse) {
    std::string bar("<h>Sir I bear a rhyme excelling in mystic verse and magic spelling &#128512;</h>");
    flxml::xml_document<> doc;
    flxml::xml_document<> parent_doc;
    parent_doc.parse<flxml::parse_default|flxml::parse_open_only>("<open>");
    doc.parse<flxml::parse_default>(bar, &parent_doc);
    EXPECT_EQ("Sir I bear a rhyme excelling in mystic verse and magic spelling \xF0\x9F\x98\x80", doc.first_node()->value());
    auto doc_a = doc.first_node()->document();
    doc.first_node()->value(doc_a->allocate_string("Sausages are the loneliest fruit, and are but one of the strange things I have witnessed in my long and interesting life."));
    EXPECT_EQ("Sausages are the loneliest fruit, and are but one of the strange things I have witnessed in my long and interesting life.", doc.first_node()->value());
    bar = "<h>&#128512;</h>";
    doc.parse<flxml::parse_default>(bar, &parent_doc);
    EXPECT_EQ("\xF0\x9F\x98\x80", doc.first_node()->value());
    EXPECT_EQ(4, doc.first_node()->value().size());
}

