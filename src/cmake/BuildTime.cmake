string(TIMESTAMP GUANAQO_BUILD_TIME UTC)
execute_process(
    COMMAND git log -1 --format=%H
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    OUTPUT_VARIABLE GUANAQO_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET)
configure_file(${CMAKE_CURRENT_LIST_DIR}/guanaqo-build-time.cpp.in
    guanaqo-build-time.cpp @ONLY)
