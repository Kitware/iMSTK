#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------
include(imstkAddLibrary)
imstk_add_library( Geometry
  H_FILES
    imstkGeometry.h
    imstkAnalyticalGeometry.h
    imstkPlane.h
    imstkSphere.h
    imstkCube.h
  CPP_FILES
    imstkGeometry.cpp
    imstkAnalyticalGeometry.cpp
    imstkPlane.cpp
    imstkSphere.cpp
    imstkCube.cpp
  LIBRARIES
    Core
  )

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
if( iMSTK_BUILD_TESTING )
  add_subdirectory( Testing )
endif()
