cmake_minimum_required(VERSION 3.24)
project(rapidxml)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
option(RAPIDXML_PERF_TESTS "Enable (very slow) performance tests" OFF)
option(RAPIDXML_SENTRY "Use Sentry (for tests only)" ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# Used on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

if (RAPIDXML_SENTRY)
    set(SENTRY_BACKEND inproc)
    add_subdirectory(sentry-native EXCLUDE_FROM_ALL)
endif(RAPIDXML_SENTRY)

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
        rapidxml_predicates.hpp
        test/xpath.cpp
        rapidxml_generator.hpp
        test/main.cc
        rapidxml_predicates.hpp
        rapidxml_tables.hpp
)
target_link_libraries(rapidxml-test PRIVATE
        GTest::gtest
)
if(RAPIDXML_SENTRY)
    target_link_libraries(rapidxml-test PRIVATE sentry)
    target_compile_definitions(rapidxml-test PRIVATE DWD_GTEST_SENTRY=1)
endif()
target_include_directories(rapidxml-test
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)
target_compile_options(rapidxml-test PRIVATE -Werror -Wall --coverage -g -O0)
target_link_options(rapidxml-test PRIVATE --coverage -g)
if (RAPIDXML_PERF_TESTS)
    message("Running performance tests")
    file(DOWNLOAD https://www.w3.org/TR/xml/REC-xml-20081126.xml ${CMAKE_CURRENT_BINARY_DIR}/REC-xml-20081126.xml)
    target_compile_definitions(rapidxml-test PRIVATE RAPIDXML_TESTING=1 RAPIDXML_PERF_TESTS=1)
else()
    message("Will skip performance tests")
    target_compile_definitions(rapidxml-test PRIVATE RAPIDXML_TESTING=1)
endif()

include(GoogleTest)
gtest_discover_tests(rapidxml-test)
