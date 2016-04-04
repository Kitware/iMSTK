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
    imstkVolumetricMesh.h
    imstkTetrahedralMesh.h
    imstkHexahedralMesh.h
	imstkGeometryMap.h
	imstkIsometricMap.h
  CPP_FILES
    imstkGeometry.cpp
    imstkPlane.cpp
    imstkSphere.cpp
    imstkCube.cpp
    imstkMesh.cpp
    imstkSurfaceMesh.cpp
    imstkVolumetricMesh.cpp
    imstkTetrahedralMesh.cpp
    imstkHexahedralMesh.cpp
	imstkGeometryMap.cpp
	imstkIsometricMap.cpp
  LIBRARIES
    Core
    VegaFEM::volumetricMesh
  )

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
if( iMSTK_BUILD_TESTING )
  add_subdirectory( Testing )
endif()
