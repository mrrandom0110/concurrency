cmake_minimum_required(VERSION 3.21.5)

project(executor)

add_compile_options(-std=c++23)

set(CMAKE_CXX_STANDARD_REQUIRED True)

option(CORO_STACK "DEBUG" ON)
option(CORO_STACK "DEBUG" OFF)
if(CORO_STACK)
    add_definitions(-DCORO_STACK=1)
else()
    add_definitions(-DCORO_STACK=0)
endif()

# find_package(Threads REQUIRED)

# SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

include(FetchContent)

message("CXX = ${CMAKE_CXX}")

add_executable(executor main.cpp)

add_subdirectory(concurrency)

target_link_libraries(executor PUBLIC exe)

target_compile_features(executor PRIVATE cxx_std_23)
