cmake_minimum_required(VERSION 3.28)

project(Tektonik)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(app)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

# SDL
add_subdirectory(external/SDL EXCLUDE_FROM_ALL)
target_link_libraries(app PRIVATE SDL3::SDL3)
add_custom_command(TARGET app POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
  $<TARGET_FILE:SDL3::SDL3>
  $<TARGET_FILE_DIR:app>)

# Add main source file
target_sources(app
    PRIVATE
    source/main.cpp
)

# Add own headers
target_include_directories(app PRIVATE "source/header")

# Add own modules
file(GLOB_RECURSE OWN_MODULE_NAMES "source/module/*.cppm")
target_sources(app
    PRIVATE
    FILE_SET own_modules TYPE CXX_MODULES FILES
    ${OWN_MODULE_NAMES}
)

# Add own implementations
file(GLOB_RECURSE OWN_IMPLEMENTATION_NAMES "source/implementation/*.cpp")
target_sources(app PRIVATE ${OWN_IMPLEMENTATION_NAMES} )


