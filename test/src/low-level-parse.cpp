//
// Created by dave on 05/07/2024.
//

#include <gtest/gtest.h>
#include <flxml.h>

TEST(Constants, Empty) {
    flxml::xml_document<> doc;
    auto empty = doc.nullstr();
    EXPECT_EQ(empty, "");
    EXPECT_EQ(empty.size(), 0);
}

TEST(Predicates, Skip) {
    std::string test_data{"<simple/>"};
    auto start = test_data.c_str();
    auto end = ++start;
    flxml::xml_document<>::skip<flxml::xml_document<>::element_name_pred,0>(end);
    EXPECT_EQ(*end, '/');
    std::string_view sv({start, end});
    EXPECT_EQ(sv, "simple");
}

TEST(PredicateBuffer, Skip) {
    std::string test_data{"<simple/>"};
    auto start = flxml::buffer_ptr(test_data);
    auto end = ++start;
    flxml::xml_document<>::skip<flxml::xml_document<>::element_name_pred,0>(end);
    EXPECT_EQ(*end, '/');
    std::string_view sv({start, end});
    EXPECT_EQ(sv, "simple");
}

TEST(Predicates, SkipAndExpand) {
    std::string test_data{"&hello;<"};
    char * start = const_cast<char *>(test_data.c_str());
    auto end = flxml::xml_document<>::skip_and_expand_character_refs<
            flxml::xml_document<>::text_pred,
            flxml::xml_document<>::text_pure_with_ws_pred,
            flxml::parse_no_entity_translation>(start);
    EXPECT_EQ(*end, '<');
}

TEST(Predicates, SkipAndExpandShort) {
    std::string test_data{"&hello;"};
    char * start = const_cast<char *>(test_data.c_str());
    auto end = flxml::xml_document<>::skip_and_expand_character_refs<
            flxml::xml_document<>::text_pred,
            flxml::xml_document<>::text_pure_with_ws_pred,
            flxml::parse_no_entity_translation>(start);
    EXPECT_EQ(*end, '\0');
}

TEST(Predicates, SkipAndExpandShorter) {
    std::string test_data{"&hell"};
    char * start = const_cast<char *>(test_data.c_str());
    auto end = flxml::xml_document<>::skip_and_expand_character_refs<
            flxml::xml_document<>::text_pred,
            flxml::xml_document<>::text_pure_with_ws_pred,
            flxml::parse_no_entity_translation>(start);
    EXPECT_EQ(*end, '\0');
}

TEST(ParseFns, ParseBom) {
    std::string test_data{"\xEF\xBB\xBF<simple/>"};
    char *start = const_cast<char *>(test_data.c_str());
    flxml::xml_document<> doc;
    doc.parse_bom<0>(start);
    EXPECT_EQ(*start, '<');
}

TEST(ParseFns, ParseBomShort) {
    std::string test_data{"\xEF\xBB\xBF"};
    char *start = const_cast<char *>(test_data.c_str());
    flxml::xml_document<> doc;
    doc.parse_bom<0>(start);
    EXPECT_EQ(*start, '\0');
}

TEST(ParseFns, ParseBomShorter) {
    std::string test_data{"\xEF\xBB"};
    char *start = const_cast<char *>(test_data.c_str());
    flxml::xml_document<> doc;
    doc.parse_bom<0>(start);
    EXPECT_EQ(*start, '\xEF');
}
