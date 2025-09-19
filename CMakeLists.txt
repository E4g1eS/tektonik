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


message("OWN_MODULES=${OWN_MODULES}")

file(GLOB_RECURSE OWN_MODULE_INTERFACE_NAMES "source/module/*.cppm")
target_sources(app
    PUBLIC
    FILE_SET own_modules TYPE CXX_MODULES FILES
    ${OWN_MODULE_INTERFACE_NAMES}
)

file(GLOB_RECURSE OWN_MODULE_IMPLEMENTATION_NAMES "source/implementation/*.cpp")
target_sources(app
    PUBLIC ${OWN_MODULE_IMPLEMENTATION_NAMES}
)