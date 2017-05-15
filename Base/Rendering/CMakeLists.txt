#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------
include(imstkAddLibrary)
imstk_add_library( Rendering
  H_FILES
    imstkVTKRenderer.h
    imstkVTKTextureDelegate.h
    RenderDelegate/imstkVTKCubeRenderDelegate.h
    RenderDelegate/imstkVTKLineMeshRenderDelegate.h
    RenderDelegate/imstkVTKPlaneRenderDelegate.h
    RenderDelegate/imstkVTKCylinderRenderDelegate.h
    RenderDelegate/imstkVTKCapsuleRenderDelegate.h
    RenderDelegate/imstkVTKRenderDelegate.h
    RenderDelegate/imstkVTKSphereRenderDelegate.h
    RenderDelegate/imstkVTKSurfaceMeshRenderDelegate.h
    RenderDelegate/imstkVTKTetrahedralMeshRenderDelegate.h
    RenderDelegate/imstkVTKHexahedralMeshRenderDelegate.h
    RenderDelegate/vtkCapsuleSource.h

  CPP_FILES
    imstkVTKRenderer.cpp
    imstkVTKTextureDelegate.cpp
    RenderDelegate/imstkVTKCubeRenderDelegate.cpp
    RenderDelegate/imstkVTKLineMeshRenderDelegate.cpp
    RenderDelegate/imstkVTKPlaneRenderDelegate.cpp
    RenderDelegate/imstkVTKCylinderRenderDelegate.cpp
    RenderDelegate/imstkVTKCapsuleRenderDelegate.cpp
    RenderDelegate/imstkVTKRenderDelegate.cpp
    RenderDelegate/imstkVTKSphereRenderDelegate.cpp
    RenderDelegate/imstkVTKSurfaceMeshRenderDelegate.cpp
    RenderDelegate/imstkVTKTetrahedralMeshRenderDelegate.cpp
    RenderDelegate/imstkVTKHexahedralMeshRenderDelegate.cpp
    RenderDelegate/vtkCapsuleSource.cpp

  SUBDIR_LIST
    RenderDelegate
  DEPENDS
    ${VTK_LIBRARIES}
    Scene
  #VERBOSE
  )

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
if( iMSTK_BUILD_TESTING )
  add_subdirectory( Testing )
endif()
