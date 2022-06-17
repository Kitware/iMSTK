#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------

include(imstkAddLibrary)
imstk_add_library(RenderingVTK
  H_FILES
    imstkRenderDelegateObjectFactory.h
    imstkVolumeRenderMaterial.h
    imstkVolumeRenderMaterialPresets.h
    imstkVTKRenderer.h
    imstkVTKTextureDelegate.h
    RenderDelegate/imstkVTKCapsuleRenderDelegate.h
    RenderDelegate/imstkVTKCylinderRenderDelegate.h
    RenderDelegate/imstkVTKFluidRenderDelegate.h
    RenderDelegate/imstkVTKHexahedralMeshRenderDelegate.h
    RenderDelegate/imstkVTKImageDataRenderDelegate.h
    RenderDelegate/imstkVTKLineMeshRenderDelegate.h
    RenderDelegate/imstkVTKPlaneRenderDelegate.h
    RenderDelegate/imstkVTKPointSetRenderDelegate.h
    RenderDelegate/imstkVTKPolyDataRenderDelegate.h
    RenderDelegate/imstkVTKOrientedBoxRenderDelegate.h
    RenderDelegate/imstkVTKRenderDelegate.h
    RenderDelegate/imstkVTKSphereRenderDelegate.h
    RenderDelegate/imstkVTKSurfaceMeshRenderDelegate.h
    RenderDelegate/imstkVTKSurfaceNormalRenderDelegate.h
    RenderDelegate/imstkVTKTetrahedralMeshRenderDelegate.h
    RenderDelegate/imstkVTKVertexLabelRenderDelegate.h
    RenderDelegate/imstkVTKVolumeRenderDelegate.h
  CPP_FILES
    imstkRenderDelegateObjectFactory.cpp
    imstkVolumeRenderMaterial.cpp
    imstkVolumeRenderMaterialPresets.cpp
    imstkVTKRenderer.cpp
    imstkVTKTextureDelegate.cpp
    RenderDelegate/imstkVTKCapsuleRenderDelegate.cpp
    RenderDelegate/imstkVTKCylinderRenderDelegate.cpp
    RenderDelegate/imstkVTKFluidRenderDelegate.cpp
    RenderDelegate/imstkVTKHexahedralMeshRenderDelegate.cpp
    RenderDelegate/imstkVTKImageDataRenderDelegate.cpp
    RenderDelegate/imstkVTKLineMeshRenderDelegate.cpp
    RenderDelegate/imstkVTKOrientedBoxRenderDelegate.cpp
    RenderDelegate/imstkVTKPlaneRenderDelegate.cpp
    RenderDelegate/imstkVTKPointSetRenderDelegate.cpp
    RenderDelegate/imstkVTKPolyDataRenderDelegate.cpp
    RenderDelegate/imstkVTKRenderDelegate.cpp
    RenderDelegate/imstkVTKSphereRenderDelegate.cpp
    RenderDelegate/imstkVTKSurfaceMeshRenderDelegate.cpp
    RenderDelegate/imstkVTKSurfaceNormalRenderDelegate.cpp
    RenderDelegate/imstkVTKTetrahedralMeshRenderDelegate.cpp
    RenderDelegate/imstkVTKVertexLabelRenderDelegate.cpp
    RenderDelegate/imstkVTKVolumeRenderDelegate.cpp
  DEPENDS
    OpenVR::OpenVR
    Animation
    RenderingCore
    VTK::RenderingContextOpenGL2
    VTK::RenderingOpenGL2
    VTK::RenderingVolumeOpenGL2
    VTK::RenderingOpenVR
    VTK::RenderingLabel
    VTK::RenderingAnnotation
    VTK::ChartsCore
  )

vtk_module_autoinit(TARGETS RenderingVTK MODULES ${VTK_LIBRARIES})

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
if( ${PROJECT_NAME}_BUILD_TESTING )
  add_subdirectory(Testing)
endif()

if( ${PROJECT_NAME}_BUILD_VISUAL_TESTING )
  add_subdirectory(VisualTesting)
endif()