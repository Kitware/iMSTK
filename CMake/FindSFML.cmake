# This script locates the SFML library
# ------------------------------------
#
# Usage
# -----
#
# When you try to locate the SFML libraries, you must specify which modules you want to use (system, window, graphics, network, audio, main).
# If none is given, the SFML_LIBRARIES variable will be empty and you'll end up linking to nothing.
# example:
#   find_package(SFML COMPONENTS graphics window system) # find the graphics, window and system modules
#
# You can enforce a specific version, either MAJOR.MINOR or only MAJOR.
# If nothing is specified, the version won't be checked (i.e. any version will be accepted).
# example:
#   find_package(SFML COMPONENTS ...)     # no specific version required
#   find_package(SFML 2 COMPONENTS ...)   # any 2.x version
#   find_package(SFML 2.4 COMPONENTS ...) # version 2.4 or greater
#
# By default, the dynamic libraries of SFML will be found. To find the static ones instead,
# you must set the SFML_STATIC_LIBRARIES variable to TRUE before calling find_package(SFML ...).
# Since you have to link yourself all the SFML dependencies when you link it statically, the following
# additional variables are defined: SFML_XXX_DEPENDENCIES and SFML_DEPENDENCIES (see their detailed
# description below).
# In case of static linking, the SFML_STATIC macro will also be defined by this script.
# example:
#   set(SFML_STATIC_LIBRARIES TRUE)
#   find_package(SFML 2 COMPONENTS network system)
#
# On Mac OS X if SFML_STATIC_LIBRARIES is not set to TRUE then by default CMake will search for frameworks unless
# CMAKE_FIND_FRAMEWORK is set to "NEVER" for example. Please refer to CMake documentation for more details.
# Moreover, keep in mind that SFML frameworks are only available as release libraries unlike dylibs which
# are available for both release and debug modes.
#
# If SFML is not installed in a standard path, you can use the SFML_ROOT CMake (or environment) variable
# to tell CMake where SFML is.
#
# Output
# ------
#
# This script defines the following variables:
# - For each specified module XXX (system, window, graphics, network, audio, main):
#   - SFML_XXX_LIBRARY_DEBUG:   the name of the debug library of the xxx module (set to SFML_XXX_LIBRARY_RELEASE is no debug version is found)
#   - SFML_XXX_LIBRARY_RELEASE: the name of the release library of the xxx module (set to SFML_XXX_LIBRARY_DEBUG is no release version is found)
#   - SFML_XXX_LIBRARY:         the name of the library to link to for the xxx module (includes both debug and optimized names if necessary)
#   - SFML_XXX_FOUND:           true if either the debug or release library of the xxx module is found
#   - SFML_XXX_DEPENDENCIES:    the list of libraries the module depends on, in case of static linking
# - SFML_LIBRARIES:    the list of all libraries corresponding to the required modules
# - SFML_FOUND:        true if all the required modules are found
# - SFML_INCLUDE_DIR:  the path where SFML headers are located (the directory containing the SFML/Config.hpp file)
# - SFML_DEPENDENCIES: the list of libraries SFML depends on, in case of static linking
#
# example:
#   find_package(SFML 2 COMPONENTS system window graphics audio REQUIRED)
#   include_directories(${SFML_INCLUDE_DIR})
#   add_executable(myapp ...)
#   target_link_libraries(myapp ${SFML_LIBRARIES})

# define the SFML_STATIC macro if static build was chosen
if(SFML_STATIC_LIBRARIES)
    add_definitions(-DSFML_STATIC)
endif()

#-----------------------------------------------------------------------------
# Find the SFML include directory
#-----------------------------------------------------------------------------
find_path(SFML_INCLUDE_DIR SFML/Config.hpp    
          PATH_SUFFIXES include)
#message(STATUS "SFML INCLUDE DIR is: ${SFML_INCLUDE_DIR}")

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


#-----------------------------------------------------------------------------
# Find library
#-----------------------------------------------------------------------------
find_library(SFML_LIBRARIES_SYS
  NAMES
    sfml-system
    sfml-system-d    
  )
find_library(SFML_LIBRARIES_MAIN
    NAMES
    sfml-main   
    sfml-main-d 
    )
find_library(SFML_LIBRARIES_AUDIO
    NAMES
    sfml-audio  
    sfml-audio-d  
    )
mark_as_advanced(SFML_LIBRARIES_SYS)
mark_as_advanced(SFML_LIBRARIES_MAIN)
mark_as_advanced(SFML_LIBRARIES_AUDIO)
        
list(APPEND SFML_LIBRARIES ${SFML_LIBRARIES_AUDIO} ${SFML_LIBRARIES_MAIN} ${SFML_LIBRARIES_SYS} )
mark_as_advanced(SFML_LIBRARIES)
        
#message(STATUS "SFML_LIBRARIES is: ${SFML_LIBRARIES}")

#-----------------------------------------------------------------------------
# Find package
#-----------------------------------------------------------------------------
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SFML
  REQUIRED_VARS
    SFML_INCLUDE_DIR
    SFML_LIBRARIES)

#-----------------------------------------------------------------------------
# If missing target, create it
#-----------------------------------------------------------------------------
if(SFML_FOUND AND NOT TARGET SFML)
  add_library(SFML INTERFACE IMPORTED)
  set_target_properties(SFML PROPERTIES
    INTERFACE_LINK_LIBRARIES "${SFML_LIBRARIES}"
    INTERFACE_INCLUDE_DIRECTORIES "${SFML_INCLUDE_DIR}"
  )
endif()


