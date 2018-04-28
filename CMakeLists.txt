cmake_minimum_required(VERSION 3.10)
project(XorList CXX)

enable_testing()
find_package(GTest REQUIRED)

set(CMAKE_CXX_STANDARD 14)
add_executable(XorList main.cpp smallfunctions.cpp allocator.cpp gtests.cpp checker.h checker.cpp test.cpp)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-DDEBUG=1)
endif()

target_include_directories(XorList PUBLIC "./include")
target_link_libraries(XorList PUBLIC GTest::GTest GTest::Main)

add_test(CommonTestsAll XorList)