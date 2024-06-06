include(GenerateExportHeader)

# Sets the default visibility to "hidden", and prevents exporting symbols from
# static libraries into shared libraries on Linux.
function(guanaqo_configure_visibility target)
    set_target_properties(${target} PROPERTIES CXX_VISIBILITY_PRESET "hidden"
                                               C_VISIBILITY_PRESET "hidden"
                                               VISIBILITY_INLINES_HIDDEN true)
    if (CMAKE_SYSTEM_NAME MATCHES "Linux")
        target_link_options(${target} PRIVATE
            $<$<LINK_LANGUAGE:C,CXX>:LINKER:--exclude-libs,ALL>)
    endif()
endfunction()

add_library(common_options INTERFACE)
if (MSVC)
    target_compile_options(common_options INTERFACE "/utf-8")
endif()
add_library(guanaqo::common_options ALIAS common_options)

# Configure the given library target with some sensible default options:
# Takes care of configuring DLL export headers, visibility flags, warning flags,
# SO version, C++ standard version, warning flags, and creation of an alias.
function(guanaqo_configure_library tgt)
    cmake_parse_arguments(GUANAQO_CFG_LIB "" "EXPORT_PREFIX" "" ${ARGN})
    cmake_path(SET export_path "")
    set(name_prefix "")
    if (DEFINED GUANAQO_CFG_LIB_EXPORT_PREFIX)
        cmake_path(APPEND export_path ${GUANAQO_CFG_LIB_EXPORT_PREFIX})
        set(name_prefix "${GUANAQO_CFG_LIB_EXPORT_PREFIX}-")
    endif()
    set_property(TARGET ${tgt} PROPERTY OUTPUT_NAME "${name_prefix}${tgt}")
    generate_export_header(${tgt} BASE_NAME "${name_prefix}${tgt}"
        EXPORT_FILE_NAME export/${export_path}/${tgt}/export.h)
    target_sources(${tgt} PUBLIC FILE_SET headers TYPE HEADERS
        BASE_DIRS   ${CMAKE_CURRENT_BINARY_DIR}/export
        FILES       ${CMAKE_CURRENT_BINARY_DIR}/export/${export_path}/${tgt}/export.h)
    set_target_properties(${tgt} PROPERTIES SOVERSION ${PROJECT_VERSION})
    guanaqo_configure_visibility(${tgt})
    target_compile_features(${tgt} PUBLIC cxx_std_20)
    if (GUANAQO_WITH_CXX_23)
        # Use C++23 to build everything, but don't require C++23 for installed
        # versions of the library (see cmake/CoreConfig.cmake.in)
        target_compile_features(${tgt} PUBLIC $<BUILD_INTERFACE:cxx_std_23>)
    endif()
    target_link_libraries(${tgt} PRIVATE guanaqo::warnings
                                 PUBLIC guanaqo::common_options)
    add_library(guanaqo::${tgt} ALIAS ${tgt})
endfunction()

# Configure the given interface library target with some sensible default
# options: C++ standard version, platform-specific flags
function(guanaqo_configure_interface_library tgt)
    target_compile_features(${tgt} INTERFACE cxx_std_20)
    if (GUANAQO_WITH_CXX_23)
        # Use C++23 to build everything, but don't require C++23 for installed
        # versions of the library (see cmake/CoreConfig.cmake.in)
        target_compile_features(${tgt} INTERFACE $<BUILD_INTERFACE:cxx_std_23>)
    endif()
    target_link_libraries(${tgt} INTERFACE guanaqo::common_options)
    add_library(guanaqo::${tgt} ALIAS ${tgt})
endfunction()

# Main library
add_library(guanaqo
    "src/demangled-typename.cpp"
    "src/dl.cpp"
    "src/dl-flags.cpp"
    "src/print.cpp"
    "src/type-erasure.cpp"
    "src/io/csv.cpp"
    "src/quadmath/quadmath-print.cpp"
)
target_sources(guanaqo PUBLIC FILE_SET headers TYPE HEADERS
    BASE_DIRS "include"
    FILES "include/guanaqo/any-ptr.hpp"
          "include/guanaqo/atomic-stop-signal.hpp"
          "include/guanaqo/callback-streambuf.hpp"
          "include/guanaqo/copyable-unique_ptr.hpp"
          "include/guanaqo/demangled-typename.hpp"
          "include/guanaqo/dl-flags.hpp"
          "include/guanaqo/duration-parse.hpp"
          "include/guanaqo/dl.hpp"
          "include/guanaqo/enumerate.hpp"
          "include/guanaqo/float.hpp"
          "include/guanaqo/from_chars-wrapper.hpp"
          "include/guanaqo/iter-adapter.hpp"
          "include/guanaqo/lifetime.hpp"
          "include/guanaqo/max-history.hpp"
          "include/guanaqo/mat-view.hpp"
          "include/guanaqo/noop-delete.hpp"
          "include/guanaqo/not-implemented.hpp"
          "include/guanaqo/possible-alias.hpp"
          "include/guanaqo/print.hpp"
          "include/guanaqo/required-method.hpp"
          "include/guanaqo/ringbuffer.hpp"
          "include/guanaqo/set-intersection.hpp"
          "include/guanaqo/string-util.hpp"
          "include/guanaqo/tag-invoke.hpp"
          "include/guanaqo/timed.hpp"
          "include/guanaqo/type-erasure.hpp"
          "include/guanaqo/type-traits.hpp"
          "include/guanaqo/io/csv.hpp"
          "include/guanaqo/quadmath/quadmath-print.hpp"
          "include/guanaqo/quadmath/quadmath.hpp"
          "include/guanaqo/eigen/span.hpp"
          "include/guanaqo/eigen/view.hpp"
)
target_link_libraries(guanaqo
    PRIVATE ${CMAKE_DL_LIBS}
    PUBLIC $<$<BOOL:${GUANAQO_WITH_QUAD_PRECISION}>:quadmath>
)
target_compile_definitions(guanaqo
    PUBLIC $<$<BOOL:${GUANAQO_WITH_QUAD_PRECISION}>:GUANAQO_WITH_QUAD_PRECISION>
)
guanaqo_configure_library(guanaqo)

# Linear algebra and sparse matrices
add_library(linalg INTERFACE)
target_sources(linalg INTERFACE FILE_SET headers TYPE HEADERS
    BASE_DIRS "include"
    FILES "include/guanaqo/linalg/config.hpp"
            "include/guanaqo/linalg/matrix-view.hpp"
            "include/guanaqo/linalg/sparse-ops.hpp"
            "include/guanaqo/linalg/sparsity-conversions.hpp"
            "include/guanaqo/linalg/sparsity.hpp"
)
guanaqo_configure_interface_library(linalg)

# Save the project version to a generated file
configure_file(cmake/guanaqo-version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/guanaqo-version.h @ONLY)
target_sources(guanaqo PUBLIC FILE_SET headers TYPE HEADERS
    BASE_DIRS "${CMAKE_CURRENT_BINARY_DIR}/include"
    FILES "${CMAKE_CURRENT_BINARY_DIR}/include/guanaqo-version.h"
)

# Build time
if (GUANAQO_WITH_ACCURATE_BUILD_TIME)
    set(GUANAQO_BUILD_TIME_CPP "${CMAKE_CURRENT_BINARY_DIR}/guanaqo-build-time.cpp")
    add_custom_target(guanaqo-build-time-generate
        BYPRODUCTS ${GUANAQO_BUILD_TIME_CPP}
        COMMAND ${CMAKE_COMMAND}
            -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/BuildTime.cmake
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    target_sources(guanaqo PRIVATE ${GUANAQO_BUILD_TIME_CPP})
    add_dependencies(guanaqo guanaqo-build-time-generate)
else()
    set(GUANAQO_BUILD_TIME_CPP "${CMAKE_CURRENT_BINARY_DIR}/guanaqo-build-time.cpp")
    if (NOT EXISTS ${GUANAQO_BUILD_TIME_CPP})
        include(cmake/BuildTime.cmake)
    endif()
    target_sources(guanaqo PRIVATE ${GUANAQO_BUILD_TIME_CPP})
endif()

# Installation
include(cmake/Install.cmake)
