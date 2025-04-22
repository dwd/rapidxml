cmake_minimum_required(VERSION 3.24)
project(rapidxml)

# Include the Conan toolchain
include(${CMAKE_CURRENT_SOURCE_DIR}/conan_toolchain.cmake)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
option(RAPIDXML_PERF_TESTS "Enable (very slow) performance tests" OFF)
option(RAPIDXML_SENTRY "Use Sentry (for tests only)" ON)

find_package(GTest)
find_package(flxml CONFIG REQUIRED)

if (RAPIDXML_SENTRY)
    set(SENTRY_BACKEND inproc)
    find_package(sentry)
endif(RAPIDXML_SENTRY)

enable_testing()
add_executable(rapidxml-test
        src/parse-simple.cpp
        src/manipulations.cpp
        src/round-trips.cpp
        src/low-level-parse.cpp
        src/perf.cpp
        src/iterators.cpp
        src/xpath.cpp
        src/main.cc
)
target_link_libraries(rapidxml-test PRIVATE
        GTest::gtest
        flxml::flxml
)
if(RAPIDXML_SENTRY)
    target_link_libraries(rapidxml-test PRIVATE sentry-native::sentry-native)
    target_compile_definitions(rapidxml-test PRIVATE DWD_GTEST_SENTRY=1)
endif()
target_include_directories(rapidxml-test
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)
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
