cmake_minimum_required(VERSION 3.28)

project(Tektonik)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(demo)

# Add main source file
target_sources(demo
    PUBLIC
    source/main.cpp
)
