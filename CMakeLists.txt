cmake_minimum_required(VERSION 3.24)
project(guanaqo
    VERSION 1.0.0
    DESCRIPTION "Utilities for scientific software"
    HOMEPAGE_URL "https://github.com/tttapa/guanaqo"
    LANGUAGES CXX
)
include(CTest)

# Detect compiler features
include(cmake/DetectFeatures.cmake)

# Options
include(CMakeDependentOption)

# Compiler support
option(GUANAQO_WITH_CXX_23
    "Enable components that require C++23" ${GUANAQO_WITH_CXX_23_DEFAULT})
# Enable/disable optional components
option(GUANAQO_WITH_TESTS
    "Build the tests" ${BUILD_TESTING})
# Enable/disable floating point types
option(GUANAQO_WITH_QUAD_PRECISION
    "Support quad-precision floating point types" Off)
# Developer options
option(GUANAQO_WARNINGS_AS_ERRORS
    "Enable -Werror or /WX" Off)
option(GUANAQO_WITH_COVERAGE
    "Generate coverage information" Off)
set(GUANAQO_DOXYFILE "Doxyfile" CACHE FILEPATH
    "The Doxyfile to use for the docs target")
option(GUANAQO_WITH_ACCURATE_BUILD_TIME
    "Update the build time on every build" On)

# Installation paths
include(GNUInstallDirs)
set(GUANAQO_INSTALL_LIBDIR "${CMAKE_INSTALL_LIBDIR}"
    CACHE PATH "Installation directory for archives and libraries")
set(GUANAQO_INSTALL_CMAKEDIR "${GUANAQO_INSTALL_LIBDIR}/cmake/guanaqo"
    CACHE PATH "Installation directory for CMake configuration files")
set(GUANAQO_INSTALL_BINDIR "${CMAKE_INSTALL_BINDIR}"
    CACHE PATH "Installation directory for binaries and DLLs")
set(GUANAQO_INSTALL_INCLUDEDIR "${CMAKE_INSTALL_INCLUDEDIR}"
    CACHE PATH "Installation directory for headers")
option(GUANAQO_STANDALONE
    "Install with relative RPATH to locate its own shared libraries" On)

# Compiler warnings
include(cmake/Warnings.cmake)
add_warnings_target(warnings ${GUANAQO_WARNINGS_AS_ERRORS})
add_library(guanaqo::warnings ALIAS warnings)

# Compiler options
if (NOT DEFINED CMAKE_CXX_EXTENSIONS)
    set(CMAKE_CXX_EXTENSIONS ${GUANAQO_WITH_QUAD_PRECISION})
endif()
if (NOT DEFINED CMAKE_C_EXTENSIONS)
    set(CMAKE_C_EXTENSIONS ${GUANAQO_WITH_QUAD_PRECISION})
endif()
if (MSVC)
    add_compile_options("/bigobj")
endif()
set(CMAKE_RELEASE_POSTFIX "")
set(CMAKE_DEBUG_POSTFIX "_d")
set(CMAKE_RELWITHDEBINFO_POSTFIX "_rd")
set(CMAKE_MINSIZEREL_POSTFIX "_rs")

# Coverage
if (GUANAQO_WITH_COVERAGE)
    add_custom_target(coverage
        ${CMAKE_CURRENT_LIST_DIR}/scripts/coverage.sh
        ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
    add_compile_options("--coverage")
    add_link_options("--coverage")
    add_dependencies(coverage guanaqo::tests)
endif()

# Documentation
find_package(Doxygen)
if (DOXYGEN_FOUND)
    add_custom_target(docs Doxygen::doxygen ${GUANAQO_DOXYFILE}
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/doxygen)
endif()

# Libraries
add_subdirectory(src)

# Tests
if (GUANAQO_WITH_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()
