//
// Created by dave on 07/07/2024.
//

#include <rapidxml.hpp>
#include <rapidxml_utils.hpp>

#include <gtest/gtest.h>
#include <numeric>
#include "rapidxml_print.hpp"
#include "rapidxml_iterators.hpp"

const auto xml_sample_file = "REC-xml-20081126.xml";

#ifdef RAPIDXML_PERF_TESTS
#define PERF_TEST() (void)0
#else
#define PERF_TEST() GTEST_SKIP() << "Skipping performance test"
#endif

TEST(Perf, Parse) {
    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::microseconds;

    PERF_TEST();
    rapidxml::file source(xml_sample_file);

    std::vector<unsigned long long> timings;
    for (auto i = 0; i != 1000; ++i) {
        rapidxml::xml_document<> doc;
        auto t1 = high_resolution_clock::now();
        doc.parse<rapidxml::parse_full>(source.data());
        auto t2 = high_resolution_clock::now();
        auto ms_int = duration_cast<microseconds>(t2 - t1);
        timings.push_back(ms_int.count());
    }
    auto total = 0ULL;
    for (auto t : timings) {
        total += t / 1000;
    }
    std::cout << "Execution time: " << total << " us\n";
}

TEST(Perf, Parse2) {
    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::microseconds;

    PERF_TEST();
    rapidxml::file source(xml_sample_file);

    std::vector<unsigned long long> timings;
    for (auto i = 0; i != 1000; ++i) {
        rapidxml::xml_document<> doc;
        std::string_view sv{source.data(), source.size() - 1}; // Drop the NUL.
        auto t1 = high_resolution_clock::now();
        doc.parse<rapidxml::parse_full>(sv);
        auto t2 = high_resolution_clock::now();
        auto ms_int = duration_cast<microseconds>(t2 - t1);
        timings.push_back(ms_int.count());
    }
    auto total = 0ULL;
    for (auto t : timings) {
        total += t / 1000;
    }
    std::cout << "Execution time: " << total << " us\n";
}

TEST(Perf, PrintClean) {
    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::microseconds;

    PERF_TEST();
    rapidxml::file source(xml_sample_file);

    std::vector<unsigned long long> timings;
    rapidxml::xml_document<> doc;
    doc.parse<rapidxml::parse_full>(source.data());
    for (auto i = 0; i != 1000; ++i) {
        std::string output;
        auto t1 = high_resolution_clock::now();
        rapidxml::print(std::back_inserter(output), doc);
        auto t2 = high_resolution_clock::now();
        auto ms_int = duration_cast<microseconds>(t2 - t1);
        timings.push_back(ms_int.count());
    }
    auto total = 0ULL;
    for (auto t : timings) {
        total += t / 1000;
    }
    std::cout << "Execution time: " << total << " us\n";
}

TEST(Perf, PrintDirty) {
    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::microseconds;

    PERF_TEST();
    rapidxml::file source(xml_sample_file);

    std::vector<unsigned long long> timings;
    rapidxml::xml_document<> doc_o;
    doc_o.parse<rapidxml::parse_full>(source.data());
    rapidxml::xml_document<> doc;
    for (auto & child : rapidxml::children(doc_o)) {
        doc.append_node(doc.clone_node(&child, true));
    }
    for (auto i = 0; i != 1000; ++i) {
        std::string output;
        auto t1 = high_resolution_clock::now();
        rapidxml::print(std::back_inserter(output), doc);
        auto t2 = high_resolution_clock::now();
        auto ms_int = duration_cast<microseconds>(t2 - t1);
        timings.push_back(ms_int.count());
    }
    auto total = 0ULL;
    for (auto t : timings) {
        total += t / 1000;
    }
    std::cout << "Execution time: " << total << " us\n";
}

