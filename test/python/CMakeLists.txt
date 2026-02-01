if (CMAKE_CROSSCOMPILING AND NOT CMAKE_CROSSCOMPILING_EMULATOR)
    find_package(Python 3.8 REQUIRED
        COMPONENTS Development.Module
        OPTIONAL_COMPONENTS Development.SABIModule)
else()
    find_package(Python 3.8 REQUIRED
        COMPONENTS Development.Module
        OPTIONAL_COMPONENTS Interpreter)
endif()
find_package(nanobind REQUIRED)

nanobind_add_module(guanaqo_matrix_view STABLE_ABI ${FREE_THREADED} "matrix_view.py.cpp")
target_compile_definitions(guanaqo_matrix_view PRIVATE
    MODULE_NAME=$<TARGET_FILE_BASE_NAME:guanaqo_matrix_view>
    VERSION_INFO="${PY_FULL_VERSION}"
)
target_link_libraries(guanaqo_matrix_view PRIVATE guanaqo::guanaqo)
set_target_properties(guanaqo_matrix_view PROPERTIES
    RELEASE_POSTFIX "" DEBUG_POSTFIX ""
    RELWITHDEBINFO_POSTFIX "" MINSIZEREL_POSTFIX "")

if (Python_Interpreter_FOUND)
    foreach (file test_matrix_view_mut.py test_matrix_view_const.py)
        file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${file}
                      INPUT ${CMAKE_CURRENT_SOURCE_DIR}/${file})
        add_test(NAME ${file}
                 COMMAND ${Python_EXECUTABLE} -m pytest ${file}
                 WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
        set_tests_properties(${file} PROPERTIES
            ENVIRONMENT "PYTHONPATH=$<TARGET_FILE_DIR:guanaqo_matrix_view>")
    endforeach()
endif()
