#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------

set(VTK_H_FILES
    VTKRenderer/imstkVTKTextStatusManager.h
    VTKRenderer/imstkVTKInteractorStyle.h
    VTKRenderer/imstkVTKScreenCaptureUtility.h
    VTKRenderer/imstkVTKViewer.h)

set(VTK_CPP_FILES
    VTKRenderer/imstkVTKTextStatusManager.cpp
    VTKRenderer/imstkVTKInteractorStyle.cpp
    VTKRenderer/imstkVTKScreenCaptureUtility.cpp
    VTKRenderer/imstkVTKViewer.cpp)

set(VULKAN_H_FILES
    VulkanRenderer/imstkVulkanInteractorStyle.h
    VulkanRenderer/imstkVulkanInteractorStyleFreeCamera.h
    VulkanRenderer/imstkVulkanScreenCaptureUtility.h
    VulkanRenderer/imstkVulkanViewer.h)

set(VULKAN_CPP_FILES
    VulkanRenderer/imstkVulkanInteractorStyle.cpp
    VulkanRenderer/imstkVulkanInteractorStyleFreeCamera.cpp
    VulkanRenderer/imstkVulkanScreenCaptureUtility.cpp
    VulkanRenderer/imstkVulkanViewer.cpp)

if(${PROJECT_NAME}_ENABLE_VR)
  list(APPEND VULKAN_H_FILES
    VulkanRenderer/imstkVulkanInteractorStyleVR.h)
  list(APPEND VULKAN_CPP_FILES
    VulkanRenderer/imstkVulkanInteractorStyleVR.cpp)
  list(APPEND VTK_H_FILES
    VTKRenderer/imstkVTKInteractorStyleVR.h)
  list(APPEND VTK_CPP_FILES
    VTKRenderer/imstkVTKInteractorStyleVR.cpp)
endif()

if( NOT iMSTK_USE_Vulkan )
  set(SIMULATIONMANAGER_H_FILES ${VTK_H_FILES})
  set(SIMULATIONMANAGER_CPP_FILES ${VTK_CPP_FILES})
  set(SIMULATIONMANAGER_SUBDIR
    VTKRenderer)
else()
  set(SIMULATIONMANAGER_H_FILES ${VULKAN_H_FILES})
  set(SIMULATIONMANAGER_CPP_FILES ${VULKAN_CPP_FILES})
  set(SIMULATIONMANAGER_SUBDIR
    VulkanRenderer)
endif()

include(imstkAddLibrary)
imstk_add_library( SimulationManager
  H_FILES
    imstkSceneManager.h
    imstkSimulationManager.h
    imstkViewer.h
    imstkScreenCaptureUtility.h
    imstkInteractorStyle.h
    ${SIMULATIONMANAGER_H_FILES}
  CPP_FILES
    imstkSceneManager.cpp
    imstkSimulationManager.cpp
    imstkViewer.cpp
    imstkScreenCaptureUtility.cpp
    ${SIMULATIONMANAGER_CPP_FILES}
  SUBDIR_LIST
    ${SIMULATIONMANAGER_SUBDIR}
  DEPENDS
    Scene
    Rendering
    ${Vulkan}
  )

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
#if( ${PROJECT_NAME}_BUILD_TESTING )
#  add_subdirectory( Testing )
#endif()
