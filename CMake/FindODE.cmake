# Finding ODE from a pre-installed directory

# Find ODE include directories and libraries
find_path(ODE_INCLUDE_DIR
    NAMES
        ode/ode.h
    PATHS
        ${ODE_ROOT_DIR}
        ${ODE_ROOT_DIR}/include
    NO_DEFAULT_PATH
)

# Create library directory hints
list(APPEND ODE_LIBRARY_HINTS ${ODE_ROOT_DIR}/lib)
# Windows compilation has special directories.
set(_ode_lib_name_win)
if(WIN32)
    list(APPEND ODE_LIBRARY_HINTS ${ODE_ROOT_DIR}/lib/ReleaseDoubleDLL)
    list(APPEND ODE_LIBRARY_HINTS ${ODE_ROOT_DIR}/lib/DebugDoubleDLL)
    set(_ode_lib_name_win ode_double)
    list(APPEND _ode_lib_name_win ode_doubled)
endif()

find_library(ODE_LIBRARY
    NAMES
        ${_ode_lib_name_win}
        ode
    HINTS
        ${ODE_LIBRARY_HINTS}
)

list(APPEND ODE_LIBRARIES ${ODE_LIBRARY})

if(NOT ODE_FIND_QUIETLY)
  message(STATUS "ODE_INCLUDE_DIR is ${ODE_INCLUDE_DIR}")
  message(STATUS "ODE_LIBRARIES is ${ODE_LIBRARIES}")
endif()

#-----------------------------------------------------------------------------
# Find package
#-----------------------------------------------------------------------------
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ODE
    REQUIRED_VARS
        ODE_INCLUDE_DIR
        ODE_LIBRARIES
    )

#-----------------------------------------------------------------------------
# If missing target, create it
#-----------------------------------------------------------------------------
if(ODE_FOUND AND NOT TARGET libodedouble)
  add_library(libodedouble INTERFACE IMPORTED)
  set_target_properties(libodedouble PROPERTIES
    INTERFACE_LINK_LIBRARIES "${ODE_LIBRARIES}"
    INTERFACE_INCLUDE_DIRECTORIES "${ODE_INCLUDE_DIR}"
   )
endif() 