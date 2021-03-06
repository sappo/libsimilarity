################################################################################
#  THIS FILE IS 100% GENERATED BY ZPROJECT; DO NOT EDIT EXCEPT EXPERIMENTALLY  #
#  Read the zproject/README.md for information about making permanent changes. #
################################################################################

if (NOT MSVC)
    include(FindPkgConfig)
    pkg_check_modules(PC_MATH "math")
    if (NOT PC_MATH_FOUND)
        pkg_check_modules(PC_MATH "math")
    endif (NOT PC_MATH_FOUND)
    if (PC_MATH_FOUND)
        # add CFLAGS from pkg-config file, e.g. draft api.
        add_definitions(${PC_MATH_CFLAGS} ${PC_MATH_CFLAGS_OTHER})
        # some libraries install the headers is a subdirectory of the include dir
        # returned by pkg-config, so use a wildcard match to improve chances of finding
        # headers and SOs.
        set(PC_MATH_INCLUDE_HINTS ${PC_MATH_INCLUDE_DIRS} ${PC_MATH_INCLUDE_DIRS}/*)
        set(PC_MATH_LIBRARY_HINTS ${PC_MATH_LIBRARY_DIRS} ${PC_MATH_LIBRARY_DIRS}/*)
    endif(PC_MATH_FOUND)
endif (NOT MSVC)

find_path (
    MATH_INCLUDE_DIRS
    NAMES math.h
    HINTS ${PC_MATH_INCLUDE_HINTS}
)

find_library (
    MATH_LIBRARIES
    NAMES m
    HINTS ${PC_MATH_LIBRARY_HINTS}
)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
    MATH
    REQUIRED_VARS MATH_LIBRARIES MATH_INCLUDE_DIRS
)
mark_as_advanced(
    MATH_FOUND
    MATH_LIBRARIES MATH_INCLUDE_DIRS
)

################################################################################
#  THIS FILE IS 100% GENERATED BY ZPROJECT; DO NOT EDIT EXCEPT EXPERIMENTALLY  #
#  Read the zproject/README.md for information about making permanent changes. #
################################################################################
