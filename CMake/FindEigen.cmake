include(imstkFind)
imstk_find_header(Eigen Eigen/Eigen eigen3)
imstk_find_header_package(Eigen)
#message(STATUS "Eigen includes : ${EIGEN_INCLUDE_DIRS}")

#-----------------------------------------------------------------------------
# Set min version to find if not set
#-----------------------------------------------------------------------------
if(NOT Eigen_FIND_VERSION)
  if(NOT Eigen_FIND_VERSION_MAJOR)
    set(Eigen_FIND_VERSION_MAJOR 3)
  endif()
  if(NOT Eigen_FIND_VERSION_MINOR)
    set(Eigen_FIND_VERSION_MINOR 0)
  endif()
  if(NOT Eigen_FIND_VERSION_PATCH)
    set(Eigen_FIND_VERSION_PATCH 0)
  endif()

  set(Eigen_FIND_VERSION "${Eigen_FIND_VERSION_MAJOR}.${Eigen_FIND_VERSION_MINOR}.${Eigen_FIND_VERSION_PATCH}")
endif()

#-----------------------------------------------------------------------------
# Macro checking version
#-----------------------------------------------------------------------------
file(READ "${EIGEN_INCLUDE_DIRS}/Eigen/src/Core/util/Macros.h" _eigen_version_header)

string(REGEX MATCH "define[ \t]+EIGEN_WORLD_VERSION[ \t]+([0-9]+)" _eigen_world_version_match "${_eigen_version_header}")
set(EIGEN_WORLD_VERSION "${CMAKE_MATCH_1}")
string(REGEX MATCH "define[ \t]+EIGEN_MAJOR_VERSION[ \t]+([0-9]+)" _eigen_major_version_match "${_eigen_version_header}")
set(EIGEN_MAJOR_VERSION "${CMAKE_MATCH_1}")
string(REGEX MATCH "define[ \t]+EIGEN_MINOR_VERSION[ \t]+([0-9]+)" _eigen_minor_version_match "${_eigen_version_header}")
set(EIGEN_MINOR_VERSION "${CMAKE_MATCH_1}")

set(Eigen_VERSION ${EIGEN_WORLD_VERSION}.${EIGEN_MAJOR_VERSION}.${EIGEN_MINOR_VERSION})
if(${Eigen_VERSION} VERSION_LESS ${Eigen_FIND_VERSION})
  set(Eigen_VERSION_OK FALSE)
else(${Eigen_VERSION} VERSION_LESS ${Eigen_FIND_VERSION})
  set(Eigen_VERSION_OK TRUE)
endif(${Eigen_VERSION} VERSION_LESS ${Eigen_FIND_VERSION})

if(NOT Eigen_VERSION_OK)
  message(STATUS "Eigen version ${Eigen_VERSION} found in ${EIGEN_INCLUDE_DIRS},"
    "but at least version ${Eigen_FIND_VERSION} is required")
endif(NOT Eigen_VERSION_OK)
