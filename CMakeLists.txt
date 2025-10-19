cmake_minimum_required(VERSION 3.28)

project(Tektonik)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# modules
set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API TRUE)
set(CMAKE_EXPERIMENTAL_CXX_MODULE_DYNDEP 1)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)

add_library(Engine)
target_compile_features(Engine PUBLIC cxx_std_23)

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

# ImGUI
target_include_directories(Engine PUBLIC "external/imgui")
file(GLOB IMGUI_SOURCES "external/imgui/*.cpp")
target_sources(Engine PUBLIC
    ${IMGUI_SOURCES}
    "external/imgui/backends/imgui_impl_vulkan.cpp"
    "external/imgui/backends/imgui_impl_sdl3.cpp"
)

# Vulkan
find_package(Vulkan REQUIRED)
message(STATUS "[Vulkan] Found package: ${Vulkan_FOUND}")
message(STATUS "[Vulkan] Version: ${Vulkan_VERSION}")
message(STATUS "[Vulkan] Include dirs: ${Vulkan_INCLUDE_DIR}")

target_link_libraries(Engine PUBLIC Vulkan::Vulkan)

## TODO
add_library(VulkanModule)
target_compile_features(VulkanModule PUBLIC cxx_std_23)
target_link_libraries(VulkanModule PUBLIC Vulkan::Vulkan)
target_include_directories(VulkanModule PUBLIC ${Vulkan_INCLUDE_DIR})
target_sources(VulkanModule
    PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES
    BASE_DIRS ${Vulkan_INCLUDE_DIR}
    FILES "${Vulkan_INCLUDE_DIR}/vulkan/vulkan.cppm"
)
target_link_libraries(Engine PUBLIC VulkanModule)
## end TODO


# Add own headers
target_include_directories(Engine PUBLIC "source/engine/header")

# Add own modules
file(GLOB_RECURSE ENGINE_MODULE_NAMES "source/engine/module/*.cppm")
target_sources(Engine
    PUBLIC
    FILE_SET engine_modules TYPE CXX_MODULES
    FILES ${ENGINE_MODULE_NAMES}
)

# Add own implementations
file(GLOB_RECURSE ENGINE_IMPLEMENTATION_NAMES "source/engine/implementation/*.cpp")
target_sources(Engine PUBLIC ${ENGINE_IMPLEMENTATION_NAMES} )

# ----------- End Engine compilation -----------

# ----------- Begin ExampleApp compilation -----------

add_executable(ExampleApp)

# Add own modules
file(GLOB_RECURSE EXAMPLEAPP_MODULE_NAMES "source/example-app/*.cppm")
target_sources(ExampleApp
    PUBLIC
    FILE_SET exampleapp_modules TYPE CXX_MODULES
    FILES ${EXAMPLEAPP_MODULE_NAMES}
)

# Add main source file
target_sources(ExampleApp
    PRIVATE
    "source/example-app/main.cpp"
)

target_link_libraries(ExampleApp PRIVATE Engine)
