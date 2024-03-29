cmake_minimum_required(VERSION 3.8)
project(rapidxml)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
FetchContent_MakeAvailable(googletest)

enable_testing()
add_executable(rapidxml-test
        test/parse-simple.cpp
        rapidxml_iterators.hpp
        rapidxml_print.hpp
        rapidxml_utils.hpp
        rapidxml.hpp
)
target_link_libraries(rapidxml-test
        GTest::gtest_main
)
target_include_directories(rapidxml-test
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)
include(GoogleTest)
gtest_discover_tests(rapidxml-test)
