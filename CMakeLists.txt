cmake_minimum_required(VERSION 3.28)

project(Tektonik)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(app)

# Add main source file
target_sources(app
    PUBLIC
    source/main.cpp
)

file(GLOB_RECURSE OWN_MODULES "source/module/*.cppm")

message("OWN_MODULES=${OWN_MODULES}")

target_sources(app
    PUBLIC
    FILE_SET all_my_modules TYPE CXX_MODULES FILES
    ${OWN_MODULES}
)
