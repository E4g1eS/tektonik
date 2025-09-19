cmake_minimum_required(VERSION 3.28)

project(Tektonik)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# module STD support
set(STD_MODULE_SUPPORT true)
if (STD_MODULE_SUPPORT)
    # Downloaded from https://raw.githubusercontent.com/stripe2933/CppStandardLibraryModule/main/cmake/EnableStandardLibraryModule.cmake
    # Check compiler support for C++23 Standard Library Module.
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "17.0.0")
        if (CMAKE_CXX_STANDARD LESS 20)
            message(FATAL_ERROR "C++20 or newer is required.")
        endif()

        set(CMAKE_CXX_STANDARD_REQUIRED YES)
        set(CMAKE_CXX_EXTENSIONS OFF)

        include(FetchContent)
        FetchContent_Declare(
            std
            URL "file://${LIBCXX_BUILD}/modules/c++/v1/"
            DOWNLOAD_EXTRACT_TIMESTAMP TRUE
            SYSTEM
        )
        FetchContent_MakeAvailable(std)

        link_libraries(std c++)
        link_libraries(std.compat c++)
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "19.36")
        if (CMAKE_CXX_STANDARD VERSION_LESS 20)
            message(FATAL_ERROR "C++20 or newer is required.")
        elseif (CMAKE_CXX_STANDARD VERSION_LESS 23 AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "19.38")
            message(FATAL_ERROR "C++23 Standard library module in C++20 is only supported with MSVC 19.38 or newer.")
        endif()

        if(DEFINED ENV{VCToolsInstallDir})
          set(VCTOOLS_INSTALL_DIR $ENV{VCToolsInstallDir})
        else()
          message(WARNING "VCToolsInstallDir environment variable is not set")
        endif()

        file(TO_CMAKE_PATH "${VCTOOLS_INSTALL_DIR}" VCTOOLS_INSTALL_DIR)

        include(FetchContent)
        FetchContent_Declare(
            std
            URL "file://${VCTOOLS_INSTALL_DIR}/modules"
            DOWNLOAD_EXTRACT_TIMESTAMP TRUE
            SYSTEM
        )
        FetchContent_MakeAvailable(std)

        add_library(std)
        target_sources(std PUBLIC
            FILE_SET CXX_MODULES
            BASE_DIRS ${std_SOURCE_DIR}
            FILES
                ${std_SOURCE_DIR}/std.ixx
                ${std_SOURCE_DIR}/std.compat.ixx
        )

        link_libraries(std)
    else()
        message(FATAL_ERROR "C++23 Standard library module is not supported with current compiler.")
    endif()
endif()

add_executable(demo)

# Add main source file
target_sources(demo
    PUBLIC
    source/main.cpp
)