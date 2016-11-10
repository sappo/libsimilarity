################################################################################
#  THIS FILE IS 100% GENERATED BY ZPROJECT; DO NOT EDIT EXCEPT EXPERIMENTALLY  #
#  Read the zproject/README.md for information about making permanent changes. #
################################################################################

if (NOT MSVC)
    include(FindPkgConfig)
    pkg_check_modules(PC_ZLIB "zlib")
    if (NOT PC_ZLIB_FOUND)
        pkg_check_modules(PC_ZLIB "zlib")
    endif (NOT PC_ZLIB_FOUND)
    if (PC_ZLIB_FOUND)
        # add CFLAGS from pkg-config file, e.g. draft api.
        add_definitions(${PC_ZLIB_CFLAGS} ${PC_ZLIB_CFLAGS_OTHER})
        # some libraries install the headers is a subdirectory of the include dir
        # returned by pkg-config, so use a wildcard match to improve chances of finding
        # headers and SOs.
        set(PC_ZLIB_INCLUDE_HINTS ${PC_ZLIB_INCLUDE_DIRS} ${PC_ZLIB_INCLUDE_DIRS}/*)
        set(PC_ZLIB_LIBRARY_HINTS ${PC_ZLIB_LIBRARY_DIRS} ${PC_ZLIB_LIBRARY_DIRS}/*)
    endif(PC_ZLIB_FOUND)
endif (NOT MSVC)

find_path (
    ZLIB_INCLUDE_DIRS
    NAMES zlib.h
    HINTS ${PC_ZLIB_INCLUDE_HINTS}
)

find_library (
    ZLIB_LIBRARIES
    NAMES zlib
    HINTS ${PC_ZLIB_LIBRARY_HINTS}
)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
    ZLIB
    REQUIRED_VARS ZLIB_LIBRARIES ZLIB_INCLUDE_DIRS
)
mark_as_advanced(
    ZLIB_FOUND
    ZLIB_LIBRARIES ZLIB_INCLUDE_DIRS
)

################################################################################
#  THIS FILE IS 100% GENERATED BY ZPROJECT; DO NOT EDIT EXCEPT EXPERIMENTALLY  #
#  Read the zproject/README.md for information about making permanent changes. #
################################################################################
