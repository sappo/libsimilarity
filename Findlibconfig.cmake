################################################################################
#  THIS FILE IS 100% GENERATED BY ZPROJECT; DO NOT EDIT EXCEPT EXPERIMENTALLY  #
#  Read the zproject/README.md for information about making permanent changes. #
################################################################################

if (NOT MSVC)
    include(FindPkgConfig)
    pkg_check_modules(PC_LIBCONFIG "libconfig")
    if (NOT PC_LIBCONFIG_FOUND)
        pkg_check_modules(PC_LIBCONFIG "libconfig")
    endif (NOT PC_LIBCONFIG_FOUND)
    if (PC_LIBCONFIG_FOUND)
        # add CFLAGS from pkg-config file, e.g. draft api.
        add_definitions(${PC_LIBCONFIG_CFLAGS} ${PC_LIBCONFIG_CFLAGS_OTHER})
        # some libraries install the headers is a subdirectory of the include dir
        # returned by pkg-config, so use a wildcard match to improve chances of finding
        # headers and SOs.
        set(PC_LIBCONFIG_INCLUDE_HINTS ${PC_LIBCONFIG_INCLUDE_DIRS} ${PC_LIBCONFIG_INCLUDE_DIRS}/*)
        set(PC_LIBCONFIG_LIBRARY_HINTS ${PC_LIBCONFIG_LIBRARY_DIRS} ${PC_LIBCONFIG_LIBRARY_DIRS}/*)
    endif(PC_LIBCONFIG_FOUND)
endif (NOT MSVC)

find_path (
    LIBCONFIG_INCLUDE_DIRS
    NAMES libconfig.h
    HINTS ${PC_LIBCONFIG_INCLUDE_HINTS}
)

find_library (
    LIBCONFIG_LIBRARIES
    NAMES config
    HINTS ${PC_LIBCONFIG_LIBRARY_HINTS}
)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
    LIBCONFIG
    REQUIRED_VARS LIBCONFIG_LIBRARIES LIBCONFIG_INCLUDE_DIRS
)
mark_as_advanced(
    LIBCONFIG_FOUND
    LIBCONFIG_LIBRARIES LIBCONFIG_INCLUDE_DIRS
)

################################################################################
#  THIS FILE IS 100% GENERATED BY ZPROJECT; DO NOT EDIT EXCEPT EXPERIMENTALLY  #
#  Read the zproject/README.md for information about making permanent changes. #
################################################################################