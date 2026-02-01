cmake_minimum_required(VERSION 3.21...4.1)
project(guanaqo-test CXX)
enable_testing()
include(GoogleTest)

option(GUANAQO_WITH_PYTHON_TESTS
    "Build the Python tests. Requires numpy and pytest." Off)

# Dependencies
find_package(GTest REQUIRED)
find_package(Eigen3 REQUIRED)
if (NOT TARGET guanaqo::guanaqo)
    find_package(guanaqo REQUIRED OPTIONAL_COMPONENTS BLAS)
endif()

add_executable(tests
    "test-any-ptr.cpp"
    "test-callback-streambuf.cpp"
    "test-duration-parse.cpp"
    "test-lut.cpp"
    "test-mat-view.cpp"
    "test-print.cpp"
    "test-set-intersection.cpp"
    "test-string-util.cpp"
    "test-tag-invoke.cpp"
    "test-type-erasure.cpp"
    "io/test-csv.cpp"
    "linalg/test-eigen.cpp"
    "linalg/test-sparsity.hpp"
    "linalg/test-sparsity-dense.cpp"
    "linalg/test-sparsity-csc.cpp"
    "linalg/test-sparsity-coo.cpp"
)
target_include_directories(tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(tests PRIVATE _CRT_SECURE_NO_WARNINGS)
target_link_libraries(tests PRIVATE
    guanaqo::guanaqo guanaqo::warnings guanaqo::linalg
    GTest::gtest_main GTest::gmock
    Eigen3::Eigen)
if (MSVC)
    target_compile_definitions(tests PRIVATE __PRETTY_FUNCTION__=__FUNCSIG__)
endif()

if (TARGET guanaqo::blas)
    target_sources(tests PRIVATE "blas/test-hl-blas-interface.cpp")
    target_link_libraries(tests PRIVATE guanaqo::blas)
endif()

option(GUANAQO_FORCE_TEST_DISCOVERY
    "Query the test executable even when cross-compiling" Off)
if (NOT CMAKE_CROSSCOMPILING OR GUANAQO_FORCE_TEST_DISCOVERY)
    gtest_discover_tests(tests DISCOVERY_TIMEOUT 60)
endif()
add_executable(guanaqo::tests ALIAS tests)

if (${CMAKE_VERSION} VERSION_GREATER_EQUAL 3.21)
    # Copy DLLs for test executable
    add_custom_command(TARGET tests PRE_LINK COMMAND
        COMMAND ${CMAKE_COMMAND} -E
            $<IF:$<BOOL:$<TARGET_RUNTIME_DLLS:tests>>,copy,true>
            $<TARGET_RUNTIME_DLLS:tests> $<TARGET_FILE_DIR:tests>
        COMMAND_EXPAND_LISTS)
endif()

if (GUANAQO_WITH_PYTHON_TESTS)
    add_subdirectory(python)
endif()
