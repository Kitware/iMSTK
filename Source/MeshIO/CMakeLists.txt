#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------

include(imstkAddLibrary)
imstk_add_library( MeshIO
  H_FILES
    imstkAssimpMeshIO.h
    imstkMeshIO.h
    imstkMshMeshIO.h
    imstkVegaMeshIO.h
    imstkVTKMeshIO.h
  CPP_FILES
    imstkAssimpMeshIO.cpp
    imstkMeshIO.cpp
    imstkMshMeshIO.cpp
    imstkVegaMeshIO.cpp
    imstkVTKMeshIO.cpp
  DEPENDS
    Common
    Geometry    
    Assimp::Assimp
    VegaFEM::volumetricMesh
    VTK::IOGeometry
    VTK::IOImage
    VTK::IOLegacy
    VTK::IOPLY
    VTK::IOXML
  )

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
if( ${PROJECT_NAME}_BUILD_TESTING )
  add_subdirectory("Testing")
endif()
