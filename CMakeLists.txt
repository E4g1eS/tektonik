cmake_minimum_required(VERSION 3.28)

project(Tektonik)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(Engine)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

# SDL
add_subdirectory(external/SDL EXCLUDE_FROM_ALL)
target_link_libraries(Engine PUBLIC SDL3::SDL3)
add_custom_command(TARGET Engine POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
  $<TARGET_FILE:SDL3::SDL3>
  $<TARGET_FILE_DIR:Engine>)

# GLM
add_subdirectory(external/glm)
target_sources(Engine
    PUBLIC
    FILE_SET CXX_MODULES
    BASE_DIRS external/glm/glm
    FILES external/glm/glm/glm.cppm
)
target_link_libraries(Engine PUBLIC glm::glm)

# Add own headers
target_include_directories(Engine PUBLIC "source/engine/header")

# Add own modules
file(GLOB_RECURSE OWN_MODULE_NAMES "source/engine/module/*.cppm")
target_sources(Engine
    PUBLIC
    FILE_SET own_modules TYPE CXX_MODULES FILES
    ${OWN_MODULE_NAMES}
)

# Add own implementations
file(GLOB_RECURSE OWN_IMPLEMENTATION_NAMES "source/engine/implementation/*.cpp")
target_sources(Engine PUBLIC ${OWN_IMPLEMENTATION_NAMES} )

# ----------- End of Engine compilation -----------

add_executable(ExampleApp)

# Add main source file
target_sources(ExampleApp
    PRIVATE
    "source/example-app/main.cpp"
)

target_link_libraries(ExampleApp PRIVATE Engine)
