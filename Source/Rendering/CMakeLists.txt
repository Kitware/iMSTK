#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------

set(VTK_H_FILES
  VTKRenderer/imstkVTKRenderer.h
  VTKRenderer/imstkVTKTextureDelegate.h
  VTKRenderer/RenderDelegate/imstkVTKOrientedBoxRenderDelegate.h
  VTKRenderer/RenderDelegate/imstkVTKLineMeshRenderDelegate.h
  VTKRenderer/RenderDelegate/imstkVTKPlaneRenderDelegate.h
  VTKRenderer/RenderDelegate/imstkVTKRenderDelegate.h
  VTKRenderer/RenderDelegate/imstkVTKSphereRenderDelegate.h
  VTKRenderer/RenderDelegate/imstkVTKImageDataRenderDelegate.h
  VTKRenderer/RenderDelegate/imstkVTKCapsuleRenderDelegate.h
  VTKRenderer/RenderDelegate/imstkVTKSurfaceMeshRenderDelegate.h
  VTKRenderer/RenderDelegate/imstkVTKSurfaceNormalRenderDelegate.h
  VTKRenderer/RenderDelegate/imstkVTKTetrahedralMeshRenderDelegate.h
  VTKRenderer/RenderDelegate/imstkVTKHexahedralMeshRenderDelegate.h
  VTKRenderer/RenderDelegate/imstkVTKCylinderRenderDelegate.h
  VTKRenderer/RenderDelegate/imstkVTKPointSetRenderDelegate.h
  VTKRenderer/RenderDelegate/imstkVTKFluidRenderDelegate.h
  VTKRenderer/RenderDelegate/imstkVTKPolyDataRenderDelegate.h
  VTKRenderer/RenderDelegate/imstkVTKVolumeRenderDelegate.h)

set(VTK_CPP_FILES
  VTKRenderer/imstkVTKRenderer.cpp
  VTKRenderer/imstkVTKTextureDelegate.cpp
  VTKRenderer/RenderDelegate/imstkVTKOrientedBoxRenderDelegate.cpp
  VTKRenderer/RenderDelegate/imstkVTKLineMeshRenderDelegate.cpp
  VTKRenderer/RenderDelegate/imstkVTKPlaneRenderDelegate.cpp
  VTKRenderer/RenderDelegate/imstkVTKRenderDelegate.cpp
  VTKRenderer/RenderDelegate/imstkVTKSphereRenderDelegate.cpp
  VTKRenderer/RenderDelegate/imstkVTKCapsuleRenderDelegate.cpp
  VTKRenderer/RenderDelegate/imstkVTKSurfaceMeshRenderDelegate.cpp
  VTKRenderer/RenderDelegate/imstkVTKSurfaceNormalRenderDelegate.cpp
  VTKRenderer/RenderDelegate/imstkVTKTetrahedralMeshRenderDelegate.cpp
  VTKRenderer/RenderDelegate/imstkVTKHexahedralMeshRenderDelegate.cpp
  VTKRenderer/RenderDelegate/imstkVTKCylinderRenderDelegate.cpp
  VTKRenderer/RenderDelegate/imstkVTKImageDataRenderDelegate.cpp
  VTKRenderer/RenderDelegate/imstkVTKPointSetRenderDelegate.cpp
  VTKRenderer/RenderDelegate/imstkVTKFluidRenderDelegate.cpp
  VTKRenderer/RenderDelegate/imstkVTKPolyDataRenderDelegate.cpp
  VTKRenderer/RenderDelegate/imstkVTKVolumeRenderDelegate.cpp)

set(RENDERING_H_FILES ${VTK_H_FILES})
set(RENDERING_CPP_FILES ${VTK_CPP_FILES})
set(RENDERING_SUBDIR
  VTKRenderer
  VTKRenderer/RenderDelegate)
set(RENDERING_DEPENDENCIES  
  Materials
  OpenVR::OpenVR)

include(imstkAddLibrary)
imstk_add_library( Rendering
  H_FILES
    imstkRenderer.h
    ${RENDERING_H_FILES}
  CPP_FILES
    ${RENDERING_CPP_FILES}
  SUBDIR_LIST
    ${RENDERING_SUBDIR}
  DEPENDS
    ${RENDERING_DEPENDENCIES}
    Scene
    Animation    
    ${VTK_LIBRARIES}
    ${RENDERING_LIBRARIES})
  #VERBOSE)

vtk_module_autoinit(TARGETS Rendering MODULES ${VTK_LIBRARIES})

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
if( ${PROJECT_NAME}_BUILD_TESTING )
  add_subdirectory(Testing)
endif()
