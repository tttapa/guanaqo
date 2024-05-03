include(${PROJECT_SOURCE_DIR}/cmake/Debug.cmake)

# Set the runtime linker/loader search paths to make guanaqo stand-alone
cmake_path(RELATIVE_PATH GUANAQO_INSTALL_LIBDIR
           BASE_DIRECTORY GUANAQO_INSTALL_BINDIR
           OUTPUT_VARIABLE GUANAQO_INSTALL_LIBRELBINDIR)

function(guanaqo_add_if_target_exists OUT)
    foreach(TGT IN LISTS ARGN)
        if (TARGET ${TGT})
            list(APPEND ${OUT} ${TGT})
        endif()
    endforeach()
    set(${OUT} ${${OUT}} PARENT_SCOPE)
endfunction()

include(CMakePackageConfigHelpers)

set(GUANAQO_INSTALLED_COMPONENTS)
macro(guanaqo_install_config PKG COMP)
    # Install the target CMake definitions
    install(EXPORT guanaqo${PKG}Targets
        FILE guanaqo${PKG}Targets.cmake
        DESTINATION "${GUANAQO_INSTALL_CMAKEDIR}"
            COMPONENT ${COMP}
        NAMESPACE guanaqo::)
    # Add all targets to the build tree export set
    export(EXPORT guanaqo${PKG}Targets
        FILE "${PROJECT_BINARY_DIR}/guanaqo${PKG}Targets.cmake"
        NAMESPACE guanaqo::)
    # Generate the config file that includes the exports
    configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PKG}Config.cmake.in"
        "${PROJECT_BINARY_DIR}/guanaqo${PKG}Config.cmake"
        INSTALL_DESTINATION "${GUANAQO_INSTALL_CMAKEDIR}"
        NO_SET_AND_CHECK_MACRO)
    write_basic_package_version_file(
        "${PROJECT_BINARY_DIR}/guanaqo${PKG}ConfigVersion.cmake"
        VERSION "${PROJECT_VERSION}"
        COMPATIBILITY SameMinorVersion)
    # Install the guanaqoConfig.cmake and guanaqoConfigVersion.cmake
    install(FILES
        "${PROJECT_BINARY_DIR}/guanaqo${PKG}Config.cmake"
        "${PROJECT_BINARY_DIR}/guanaqo${PKG}ConfigVersion.cmake"
        DESTINATION "${GUANAQO_INSTALL_CMAKEDIR}"
            COMPONENT ${COMP})
    list(APPEND GUANAQO_OPTIONAL_COMPONENTS ${PKG})
endmacro()

macro(guanaqo_install_cmake FILES COMP)
    # Install a CMake script
    install(FILES ${FILES}
        DESTINATION "${GUANAQO_INSTALL_CMAKEDIR}"
            COMPONENT ${COMP})
endmacro()

set(GUANAQO_INSTALLED_TARGETS_MSG "\nSummary of guanaqo components and targets to install:\n\n")

# Install the guanaqo core libraries
set(GUANAQO_CORE_TARGETS guanaqo)
guanaqo_add_if_target_exists(GUANAQO_CORE_TARGETS linalg)
if (GUANAQO_CORE_TARGETS)
    install(TARGETS warnings common_options ${GUANAQO_CORE_TARGETS}
        EXPORT guanaqoCoreTargets
        RUNTIME DESTINATION "${GUANAQO_INSTALL_BINDIR}"
            COMPONENT lib
        LIBRARY DESTINATION "${GUANAQO_INSTALL_LIBDIR}"
            COMPONENT lib
            NAMELINK_COMPONENT dev
        ARCHIVE DESTINATION "${GUANAQO_INSTALL_LIBDIR}"
            COMPONENT dev
        FILE_SET headers DESTINATION "${GUANAQO_INSTALL_INCLUDEDIR}"
            COMPONENT dev)
    guanaqo_install_config(Core dev)
    list(JOIN GUANAQO_CORE_TARGETS ", " TGTS)
    string(APPEND GUANAQO_INSTALLED_TARGETS_MSG " * Core:  ${TGTS}\n")
    list(APPEND GUANAQO_INSTALL_TARGETS ${GUANAQO_CORE_TARGETS})
endif()

# Install the debug files
foreach(target IN LISTS GUANAQO_CORE_TARGETS)
    get_target_property(target_type ${target} TYPE)
    if (${target_type} STREQUAL "SHARED_LIBRARY")
        guanaqo_install_debug_syms(${target} debug
                                  ${GUANAQO_INSTALL_LIBDIR}
                                  ${GUANAQO_INSTALL_BINDIR})
    elseif (${target_type} STREQUAL "EXECUTABLE")
        guanaqo_install_debug_syms(${target} debug
                                  ${GUANAQO_INSTALL_BINDIR}
                                  ${GUANAQO_INSTALL_BINDIR})
    endif()
endforeach()

# Make stand-alone
if (GUANAQO_STANDALONE)
    foreach(target IN LISTS GUANAQO_CORE_TARGETS)
        set_target_properties(${TGT} PROPERTIES
            INSTALL_RPATH "$ORIGIN;$ORIGIN/${GUANAQO_INSTALL_LIBRELBINDIR}")
    endforeach()
endif()

# Generate the main config file
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in"
    "${PROJECT_BINARY_DIR}/guanaqoConfig.cmake"
    INSTALL_DESTINATION "${GUANAQO_INSTALL_CMAKEDIR}"
    NO_SET_AND_CHECK_MACRO)
write_basic_package_version_file(
    "${PROJECT_BINARY_DIR}/guanaqoConfigVersion.cmake"
    VERSION "${PROJECT_VERSION}"
    COMPATIBILITY SameMinorVersion)
# Install the main guanaqoConfig.cmake and guanaqoConfigVersion.cmake files
install(FILES
    "${PROJECT_BINARY_DIR}/guanaqoConfig.cmake"
    "${PROJECT_BINARY_DIR}/guanaqoConfigVersion.cmake"
    DESTINATION "${GUANAQO_INSTALL_CMAKEDIR}"
        COMPONENT dev)

# Print the components and targets we're going to install
message(${GUANAQO_INSTALLED_TARGETS_MSG})
