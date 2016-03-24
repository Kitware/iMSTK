#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------
include(imstkAddLibrary)
imstk_add_library( Geometry
  H_FILES
    imstkGeometry.h
    imstkPlane.h
    imstkSphere.h
    imstkCube.h
    imstkMesh.h
    imstkSurfaceMesh.h
  CPP_FILES
    imstkGeometry.cpp
    imstkPlane.cpp
    imstkSphere.cpp
    imstkCube.cpp
    imstkMesh.cpp
    imstkSurfaceMesh.cpp
  LIBRARIES
    Core
  )

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
if( iMSTK_BUILD_TESTING )
  add_subdirectory( Testing )
endif()
