cmake_minimum_required(VERSION 3.8)
project(rapidxml)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
option(RAPIDXML_PERF_TESTS "Enable (very slow) performance tests" OFF)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# Used on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

enable_testing()
add_executable(rapidxml-test
        test/parse-simple.cpp
        rapidxml_iterators.hpp
        rapidxml_print.hpp
        rapidxml_utils.hpp
        rapidxml.hpp
        test/manipulations.cpp
        test/round-trips.cpp
        test/low-level-parse.cpp
        test/perf.cpp
        rapidxml_wrappers.hpp
        test/iterators.cpp
)
target_link_libraries(rapidxml-test
        GTest::gtest_main
)
target_include_directories(rapidxml-test
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)
if (${RAPIDXML_PERF_TESTS})
    target_compile_definitions(rapidxml-test PRIVATE RAPIDXML_TESTING=1)
else()
    file(DOWNLOAD https://www.w3.org/TR/xml/REC-xml-20081126.xml ${CMAKE_CURRENT_BINARY_DIR}/REC-xml-20081126.xml)
    target_compile_definitions(rapidxml-test PRIVATE RAPIDXML_TESTING=1 RAPIDXML_PERF_TESTS=1)
endif()

include(GoogleTest)
gtest_discover_tests(rapidxml-test)
