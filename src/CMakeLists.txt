function(guanaqo_configure_visibility target)
    set_target_properties(${target} PROPERTIES CXX_VISIBILITY_PRESET "hidden"
                                               C_VISIBILITY_PRESET "hidden"
                                               VISIBILITY_INLINES_HIDDEN true)
    if (CMAKE_SYSTEM_NAME MATCHES "Linux")
        target_link_options(${target} PRIVATE
            $<$<LINK_LANGUAGE:C,CXX>:LINKER:--exclude-libs,ALL>)
    endif()
endfunction()

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
          "include/guanaqo/copyable_unique_ptr.hpp"
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
)

target_compile_features(guanaqo PUBLIC cxx_std_20)
if (GUANAQO_WITH_CXX_23)
# Use C++23 to build everything, but don't require C++23 for installed
# versions of the library (see cmake/CoreConfig.cmake.in)
target_compile_features(guanaqo PUBLIC $<BUILD_INTERFACE:cxx_std_23>)
endif()
target_link_libraries(guanaqo
    PRIVATE
        warnings
        ${CMAKE_DL_LIBS}
    PUBLIC
        $<$<BOOL:${GUANAQO_WITH_QUAD_PRECISION}>:quadmath>
)
target_compile_definitions(guanaqo PUBLIC
    $<$<BOOL:${GUANAQO_WITH_QUAD_PRECISION}>:GUANAQO_WITH_QUAD_PRECISION>
)
if (MSVC)
    target_compile_options(guanaqo PUBLIC "/utf-8")
endif()
guanaqo_configure_visibility(guanaqo)
set_target_properties(guanaqo PROPERTIES SOVERSION ${PROJECT_VERSION})
add_library(guanaqo::guanaqo ALIAS guanaqo)

# DLL import/export
include(GenerateExportHeader)
generate_export_header(guanaqo EXPORT_FILE_NAME export/guanaqo/export.h)
target_include_directories(guanaqo PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/export>)

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
