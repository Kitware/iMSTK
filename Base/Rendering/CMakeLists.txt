#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------
include(imstkAddLibrary)
imstk_add_library( Rendering
  H_FILES
    imstkVTKRenderer.h
    RenderDelegate/imstkVTKCubeRenderDelegate.h
    RenderDelegate/imstkVTKLineMeshRenderDelegate.h
    RenderDelegate/imstkVTKPlaneRenderDelegate.h
    RenderDelegate/imstkVTKRenderDelegate.h
    RenderDelegate/imstkVTKSphereRenderDelegate.h
    RenderDelegate/imstkVTKSurfaceMeshRenderDelegate.h
    RenderDelegate/imstkVTKTetrahedralMeshRenderDelegate.h
  CPP_FILES
    imstkVTKRenderer.cpp
    RenderDelegate/imstkVTKCubeRenderDelegate.cpp
    RenderDelegate/imstkVTKLineMeshRenderDelegate.cpp
    RenderDelegate/imstkVTKPlaneRenderDelegate.cpp
    RenderDelegate/imstkVTKRenderDelegate.cpp
    RenderDelegate/imstkVTKSphereRenderDelegate.cpp
    RenderDelegate/imstkVTKSurfaceMeshRenderDelegate.cpp
    RenderDelegate/imstkVTKTetrahedralMeshRenderDelegate.cpp
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
