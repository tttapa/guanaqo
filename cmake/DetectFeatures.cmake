# Determine compiler's C++23 support
set(GUANAQO_WITH_CXX_23_DEFAULT Off)
if ("cxx_std_23" IN_LIST CMAKE_CXX_COMPILE_FEATURES)
    set(GUANAQO_WITH_CXX_23_DEFAULT On)
endif()

# Detect optional platform APIs.
include(CheckCXXSymbolExists)
include(CMakePushCheckState)

check_cxx_symbol_exists(pthread_self "pthread.h" GUANAQO_HAVE_PTHREAD_SELF)

cmake_push_check_state(RESET)
set(CMAKE_REQUIRED_LIBRARIES ${CMAKE_DL_LIBS})
check_cxx_symbol_exists(dlopen "dlfcn.h" GUANAQO_HAVE_DLFCN)
cmake_pop_check_state()
