#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------

set(VTK_H_FILES
    VTKRenderer/imstkVTKRenderer.h
    VTKRenderer/imstkVTKTextureDelegate.h
    VTKRenderer/RenderDelegate/imstkVTKCubeRenderDelegate.h
    VTKRenderer/RenderDelegate/imstkVTKLineMeshRenderDelegate.h
    VTKRenderer/RenderDelegate/imstkVTKPlaneRenderDelegate.h
    VTKRenderer/RenderDelegate/imstkVTKRenderDelegate.h
    VTKRenderer/RenderDelegate/imstkVTKSphereRenderDelegate.h
    VTKRenderer/RenderDelegate/imstkVTKImageDataRenderDelegate.h
    VTKRenderer/RenderDelegate/imstkVTKCapsuleRenderDelegate.h
    VTKRenderer/RenderDelegate/imstkVTKSurfaceMeshRenderDelegate.h
    VTKRenderer/RenderDelegate/imstkVTKTetrahedralMeshRenderDelegate.h
    VTKRenderer/RenderDelegate/imstkVTKHexahedralMeshRenderDelegate.h
    VTKRenderer/RenderDelegate/imstkVTKCylinderRenderDelegate.h
    VTKRenderer/RenderDelegate/imstkVTKdebugTrianglesRenderDelegate.h  
    VTKRenderer/RenderDelegate/imstkVTKdebugLinesRenderDelegate.h 
    VTKRenderer/RenderDelegate/imstkVTKdebugPointsRenderDelegate.h   
    VTKRenderer/RenderDelegate/imstkVTKPointSetRenderDelegate.h
	VTKRenderer/RenderDelegate/imstkVTKFluidRenderDelegate.h)

set(VTK_CPP_FILES
    VTKRenderer/imstkVTKRenderer.cpp
    VTKRenderer/imstkVTKTextureDelegate.cpp
    VTKRenderer/RenderDelegate/imstkVTKCubeRenderDelegate.cpp
    VTKRenderer/RenderDelegate/imstkVTKLineMeshRenderDelegate.cpp
    VTKRenderer/RenderDelegate/imstkVTKPlaneRenderDelegate.cpp
    VTKRenderer/RenderDelegate/imstkVTKRenderDelegate.cpp
    VTKRenderer/RenderDelegate/imstkVTKSphereRenderDelegate.cpp
    VTKRenderer/RenderDelegate/imstkVTKCapsuleRenderDelegate.cpp
    VTKRenderer/RenderDelegate/imstkVTKSurfaceMeshRenderDelegate.cpp
    VTKRenderer/RenderDelegate/imstkVTKTetrahedralMeshRenderDelegate.cpp
    VTKRenderer/RenderDelegate/imstkVTKHexahedralMeshRenderDelegate.cpp
    VTKRenderer/RenderDelegate/imstkVTKCylinderRenderDelegate.cpp
    VTKRenderer/RenderDelegate/imstkVTKImageDataRenderDelegate.cpp
    VTKRenderer/RenderDelegate/imstkVTKdebugTrianglesRenderDelegate.cpp
    VTKRenderer/RenderDelegate/imstkVTKdebugLinesRenderDelegate.cpp
    VTKRenderer/RenderDelegate/imstkVTKdebugPointsRenderDelegate.cpp
    VTKRenderer/RenderDelegate/imstkVTKPointSetRenderDelegate.cpp
	VTKRenderer/RenderDelegate/imstkVTKFluidRenderDelegate.cpp)

set(VULKAN_H_FILES
    VulkanRenderer/imstkVulkanRenderer.h
    VulkanRenderer/RenderDelegate/imstkVulkanCapsuleRenderDelegate.h
    VulkanRenderer/RenderDelegate/imstkVulkanCubeRenderDelegate.h
    VulkanRenderer/RenderDelegate/imstkVulkanDecalRenderDelegate.h
    VulkanRenderer/RenderDelegate/imstkVulkanLineMeshRenderDelegate.h
    VulkanRenderer/RenderDelegate/imstkVulkanParticleRenderDelegate.h
    VulkanRenderer/RenderDelegate/imstkVulkanPlaneRenderDelegate.h
    VulkanRenderer/RenderDelegate/imstkVulkanRenderDelegate.h
    VulkanRenderer/RenderDelegate/imstkVulkanSphereRenderDelegate.h
    VulkanRenderer/RenderDelegate/imstkVulkanSurfaceMeshRenderDelegate.h
    VulkanRenderer/imstkVulkanMaterialDelegate.h
    VulkanRenderer/imstkVulkanTextureDelegate.h
    VulkanRenderer/imstkVulkanMemoryManager.h
    VulkanRenderer/imstkVulkanResources.h
    VulkanRenderer/imstkVulkanValidation.h
    VulkanRenderer/imstkVulkanBuffer.h
    VulkanRenderer/imstkVulkanUniformBuffer.h
    VulkanRenderer/imstkVulkanVertexBuffer.h
    VulkanRenderer/imstkVulkanFramebuffer.h
    VulkanRenderer/imstkVulkanUtilities.h
    VulkanRenderer/imstkVulkanRenderPassGenerator.h
    VulkanRenderer/PostProcessing/imstkVulkanPostProcess.h
    VulkanRenderer/PostProcessing/imstkVulkanPostProcessingChain.h)

set(VULKAN_CPP_FILES
    VulkanRenderer/imstkVulkanRenderer.cpp
    VulkanRenderer/RenderDelegate/imstkVulkanCapsuleRenderDelegate.cpp
    VulkanRenderer/RenderDelegate/imstkVulkanCubeRenderDelegate.cpp
    VulkanRenderer/RenderDelegate/imstkVulkanDecalRenderDelegate.cpp
    VulkanRenderer/RenderDelegate/imstkVulkanLineMeshRenderDelegate.cpp
    VulkanRenderer/RenderDelegate/imstkVulkanParticleRenderDelegate.cpp
    VulkanRenderer/RenderDelegate/imstkVulkanPlaneRenderDelegate.cpp
    VulkanRenderer/RenderDelegate/imstkVulkanRenderDelegate.cpp
    VulkanRenderer/RenderDelegate/imstkVulkanSphereRenderDelegate.cpp
    VulkanRenderer/RenderDelegate/imstkVulkanSurfaceMeshRenderDelegate.cpp
    VulkanRenderer/imstkVulkanMaterialDelegate.cpp
    VulkanRenderer/imstkVulkanTextureDelegate.cpp
    VulkanRenderer/imstkVulkanMemoryManager.cpp
    VulkanRenderer/imstkVulkanUniformBuffer.cpp
    VulkanRenderer/imstkVulkanVertexBuffer.cpp
    VulkanRenderer/imstkVulkanFramebuffer.cpp
    VulkanRenderer/imstkVulkanRenderPassGenerator.cpp
    VulkanRenderer/PostProcessing/imstkVulkanPostProcess.cpp
    VulkanRenderer/PostProcessing/imstkVulkanPostProcessingChain.cpp)

if( NOT iMSTK_USE_Vulkan )
  set(RENDERING_H_FILES ${VTK_H_FILES})
  set(RENDERING_CPP_FILES ${VTK_CPP_FILES})
  set(RENDERING_SUBDIR
    VTKRenderer
    VTKRenderer/RenderDelegate)
  set(RENDERING_DEPENDENCIES
    #GUIOverlay
    Materials)
else()
  set(RENDERING_H_FILES ${VULKAN_H_FILES})
  set(RENDERING_CPP_FILES ${VULKAN_CPP_FILES})
  set(RENDERING_SUBDIR
    VulkanRenderer
    VulkanRenderer/RenderDelegate
    VulkanRenderer/PostProcessing)
  set(RENDERING_DEPENDENCIES
    GUIOverlay
    Materials)
  set(RENDERING_LIBRARIES
    ${Vulkan}
    glfw
    gli)
  if (iMSTK_ENABLE_VR)
    list(APPEND RENDERING_DEPENDENCIES openvr)
  endif()
endif()

include(imstkAddLibrary)
imstk_add_library( Rendering
  H_FILES
    imstkRenderer.h
    ${RENDERING_H_FILES}
    imstkVtkCapsuleSource.h
  CPP_FILES
    imstkRenderer.cpp
    ${RENDERING_CPP_FILES}
    imstkVtkCapsuleSource.cpp
  SUBDIR_LIST
    ${RENDERING_SUBDIR}
  DEPENDS
    ${RENDERING_DEPENDENCIES}
    Scene
    Animation
    glm
    ${VTK_LIBRARIES}
    ${RENDERING_LIBRARIES}
  #VERBOSE
  )

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
#if( ${PROJECT_NAME}_BUILD_TESTING )
#  add_subdirectory( Testing )
#endif()
