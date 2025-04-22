//
// Created by dave on 29/07/2024.
//

#include <gtest/gtest.h>
#include <flxml/predicates.h>

TEST(XPath, parse) {
    std::string xpath_string = "//";
    std::string_view sv{xpath_string};
    auto xp = flxml::xpath<>::parse(sv);
    EXPECT_EQ(sv.length(), 0);
    EXPECT_NE(xp.get(), nullptr);
    EXPECT_EQ(xp->chain().size(), 1);
}

TEST(XPath, parse2) {
    std::string xpath_string = "//child";
    std::string_view sv{xpath_string};
    auto xp = flxml::xpath<>::parse(sv);
    EXPECT_EQ(sv.length(), 0);
    EXPECT_NE(xp.get(), nullptr);
    EXPECT_EQ(xp->chain().size(), 2);
}

TEST(XPath, parse1) {
    std::string xpath_string = "/child";
    std::string_view sv{xpath_string};
    auto xp = flxml::xpath<>::parse(sv);
    EXPECT_EQ(sv.length(), 0);
    EXPECT_NE(xp.get(), nullptr);
    EXPECT_EQ(xp->chain().size(), 2);
}

TEST(XPath, parse3) {
    std::string xpath_string = "//child[another/element]/something";
    std::string_view sv{xpath_string};
    auto xp = flxml::xpath<>::parse(sv);
    EXPECT_EQ(sv.length(), 0);
    EXPECT_NE(xp.get(), nullptr);
    EXPECT_EQ(xp->chain().size(), 4);
    ASSERT_EQ(xp->chain()[1]->contexts().size(), 1);
    EXPECT_EQ(xp->chain()[1]->contexts().begin()->get()->chain().size(), 4);
}

TEST(XPath, parse4) {
    std::string xpath_string = "";
    std::string_view sv{xpath_string};
    EXPECT_THROW(
        flxml::xpath<>::parse(sv),
        std::runtime_error
    );
}


TEST(XPath, parse_attr) {
    std::string xpath_string = "//child[@foo='bar']/something";
    std::string_view sv{xpath_string};
    auto xp = flxml::xpath<>::parse(sv);
    EXPECT_EQ(sv.length(), 0);
    EXPECT_NE(xp.get(), nullptr);
    EXPECT_EQ(xp->chain().size(), 4);
    ASSERT_EQ(xp->chain()[1]->contexts().size(), 1);
    EXPECT_EQ(xp->chain()[1]->contexts().begin()->get()->chain().size(), 1);
}

TEST(XPath, parse_text) {
    std::string xpath_string = "//child[text()='bar']/something";
    std::string_view sv{xpath_string};
    auto xp = flxml::xpath<>::parse(sv);
    EXPECT_EQ(sv.length(), 0);
    EXPECT_NE(xp.get(), nullptr);
    EXPECT_EQ(xp->chain().size(), 4);
    ASSERT_EQ(xp->chain()[1]->contexts().size(), 1);
    EXPECT_EQ(xp->chain()[1]->contexts().begin()->get()->chain().size(), 1);
}

TEST(XPathFirst, simple_all) {
    flxml::xml_document<> doc;
    doc.parse<flxml::parse_fastest>("<simple><child/></simple>");
    std::string xpath = "//";
    std::string_view sv{xpath};
    auto xp = flxml::xpath<>::parse(sv);
    auto r =  xp->first(doc);
    ASSERT_TRUE(r);
    EXPECT_EQ(r->type(), flxml::node_type::node_document);
}

TEST(XPathFirst, simple_any) {
    flxml::xml_document<> doc;
    doc.parse<flxml::parse_fastest>("<simple><child/></simple>");
    std::string xpath = "//child";
    std::string_view sv{xpath};
    auto xp = flxml::xpath<>::parse(sv);
    auto r =  xp->first(doc);
    ASSERT_TRUE(r);
    EXPECT_EQ(r->name(), "child");
}

TEST(XPathFirst, simple_sub) {
    flxml::xml_document<> doc;
    doc.parse<flxml::parse_fastest>("<simple><child/></simple>");
    std::string xpath = "//[child]";
    std::string_view sv{xpath};
    auto xp = flxml::xpath<>::parse(sv);
    auto r =  xp->first(doc);
    ASSERT_TRUE(r);
    EXPECT_EQ(r->name(), "simple");
}

TEST(XPathFirst, simple_attr) {
    flxml::xml_document<> doc;
    doc.parse<flxml::parse_fastest>("<simple><child attr='val1'>foo</child><child attr='val2'>bar</child></simple>");
    std::string xpath = "//child[@attr='val2']";
    std::string_view sv{xpath};
    auto xp = flxml::xpath<>::parse(sv);
    auto r =  xp->first(doc);
    ASSERT_TRUE(r);
    EXPECT_EQ(r->name(), "child");
    EXPECT_EQ(r->value(), "bar");
}

TEST(XPathFirst, simple_text) {
    flxml::xml_document<> doc;
    doc.parse<flxml::parse_fastest>("<simple><child attr='val1'>foo</child><child attr='val2'>bar</child></simple>");
    auto xp = flxml::xpath<>::parse("//child[text()='bar']");
    auto r =  xp->first(doc);
    ASSERT_TRUE(r);
    EXPECT_EQ(r->name(), "child");
    EXPECT_EQ(r->value(), "bar");
}

TEST(XPathNS, simple_text) {
    flxml::xml_document<> doc;
    doc.parse<flxml::parse_fastest>("<p1:simple xmlns:p1='p2'><p1:child attr='val1'>foo</p1:child><p1:child attr='val2'>bar</p1:child></p1:simple>");
    auto xp = flxml::xpath<>::parse("//child[text()='bar']");
    auto r =  xp->first(doc);
    ASSERT_TRUE(r);
    EXPECT_EQ(r->name(), "child");
    EXPECT_EQ(r->value(), "bar");
}

TEST(XPathNS, xmlns_text) {
    flxml::xml_document<> doc;
    doc.parse<flxml::parse_fastest>("<p1:simple xmlns:p1='p2'><p1:child attr='val1'>foo</p1:child><p1:child attr='val2'>bar</p1:child></p1:simple>");
    std::map<std::string,std::string> xmlns = {
            {"x1", "p2"},
            {"x2", "p1"}
    };
    auto xp = flxml::xpath<>::parse(xmlns,"//x1:child[text()='bar']");
    auto r =  xp->first(doc);
    ASSERT_TRUE(r);
    EXPECT_EQ(r->name(), "child");
    EXPECT_EQ(r->value(), "bar");
}

TEST(XPathNS, xmlns_both) {
    flxml::xml_document<> doc;
    doc.parse<flxml::parse_fastest>("<p1:simple xmlns:p1='p2'><p1:child attr='val1'>foo</p1:child><p1:child attr='val2'>bar</p1:child></p1:simple>");
    std::map<std::string,std::string> xmlns = {
            {"x1", "p2"},
            {"x2", "p1"}
    };
    auto xp = flxml::xpath<>::parse(xmlns,"//x1:child[text()='bar'][@attr='val2']");
    auto r =  xp->first(doc);
    ASSERT_TRUE(r);
    EXPECT_EQ(r->name(), "child");
    EXPECT_EQ(r->value(), "bar");
}

TEST(XPathNS, xmlns_text_miss) {
    flxml::xml_document<> doc;
    doc.parse<flxml::parse_fastest>("<p1:simple xmlns:p1='p2'><p1:child attr='val1'>foo</p1:child><p1:child attr='val2'>bar</p1:child></p1:simple>");
    std::map<std::string,std::string> xmlns = {
            {"x1", "p2"},
            {"x2", "p1"}
    };
    auto xp = flxml::xpath<>::parse(xmlns,"//x2:child[text()='bar']");
    auto r =  xp->first(doc);
    ASSERT_FALSE(r);
}
