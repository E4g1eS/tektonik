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

target_include_directories(app PUBLIC "source/header")

file(GLOB_RECURSE OWN_MODULE_NAMES "source/module/*.cppm")
target_sources(app
    PUBLIC
    FILE_SET own_modules TYPE CXX_MODULES FILES
    ${OWN_MODULE_NAMES}
)

file(GLOB_RECURSE OWN_HEADER_NAMES "source/header/*.hpp")
target_sources(app
    PUBLIC
    FILE_SET own_headers TYPE HEADERS FILES
    ${OWN_HEADER_NAMES}
)

file(GLOB_RECURSE OWN_IMPLEMENTATION_NAMES "source/implementation/*.cpp")
target_sources(app PUBLIC ${OWN_IMPLEMENTATION_NAMES} )


