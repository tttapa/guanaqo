# Determine compiler's C++23 support
set(GUANAQO_WITH_CXX_23_DEFAULT Off)
if ("cxx_std_23" IN_LIST CMAKE_CXX_COMPILE_FEATURES)
    set(GUANAQO_WITH_CXX_23_DEFAULT On)
endif()
