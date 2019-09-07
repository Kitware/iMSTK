include(imstkFind)
#-----------------------------------------------------------------------------
# Find All Headers and Libraries for SFML
#-----------------------------------------------------------------------------

imstk_find_header(SFML Config.hpp SFML)
#SMFL has their own debug postfix
imstk_find_libary(SFML sfml-system "-d")
imstk_find_libary(SFML sfml-main "-d")
imstk_find_libary(SFML sfml-audio "-d")
imstk_find_package(SFML)

#message(STATUS "SFML include : ${SFML_INCLUDE_DIRS}")
#message(STATUS "SFML libraries : ${SFML_LIBRARIES}")

#-----------------------------------------------------------------------------
# check the version number
#-----------------------------------------------------------------------------
set(SFML_VERSION_OK TRUE)
if(SFML_FIND_VERSION AND SFML_INCLUDE_DIR)
    # extract the major and minor version numbers from SFML/Config.hpp
    # we have to handle framework a little bit differently:
    if("${SFML_INCLUDE_DIR}" MATCHES "SFML.framework")
        set(SFML_CONFIG_HPP_INPUT "${SFML_INCLUDE_DIR}/Headers/Config.hpp")
    else()
        set(SFML_CONFIG_HPP_INPUT "${SFML_INCLUDE_DIR}/SFML/Config.hpp")
    endif()
    FILE(READ "${SFML_CONFIG_HPP_INPUT}" SFML_CONFIG_HPP_CONTENTS)
    STRING(REGEX REPLACE ".*#define SFML_VERSION_MAJOR ([0-9]+).*" "\\1" SFML_VERSION_MAJOR "${SFML_CONFIG_HPP_CONTENTS}")
    STRING(REGEX REPLACE ".*#define SFML_VERSION_MINOR ([0-9]+).*" "\\1" SFML_VERSION_MINOR "${SFML_CONFIG_HPP_CONTENTS}")
    STRING(REGEX REPLACE ".*#define SFML_VERSION_PATCH ([0-9]+).*" "\\1" SFML_VERSION_PATCH "${SFML_CONFIG_HPP_CONTENTS}")
    if (NOT "${SFML_VERSION_PATCH}" MATCHES "^[0-9]+$")
        set(SFML_VERSION_PATCH 0)
    endif()
    math(EXPR SFML_REQUESTED_VERSION "${SFML_FIND_VERSION_MAJOR} * 10000 + ${SFML_FIND_VERSION_MINOR} * 100 + ${SFML_FIND_VERSION_PATCH}")

    # if we could extract them, compare with the requested version number
    if (SFML_VERSION_MAJOR)
        # transform version numbers to an integer
        math(EXPR SFML_VERSION "${SFML_VERSION_MAJOR} * 10000 + ${SFML_VERSION_MINOR} * 100 + ${SFML_VERSION_PATCH}")

        # compare them
        if(SFML_VERSION LESS SFML_REQUESTED_VERSION)
            set(SFML_VERSION_OK FALSE)
        endif()
    else()
        # SFML version is < 2.0
        if (SFML_REQUESTED_VERSION GREATER 10900)
            set(SFML_VERSION_OK FALSE)
            set(SFML_VERSION_MAJOR 1)
            set(SFML_VERSION_MINOR x)
            set(SFML_VERSION_PATCH x)
        endif()
    endif()
endif()

