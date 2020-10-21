#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------

set(VTK_H_FILES
    VTKRenderer/imstkVTKTextStatusManager.h
    VTKRenderer/imstkVTKInteractorStyle.h
    VTKRenderer/imstkVTKScreenCaptureUtility.h
    VTKRenderer/imstkAbstractVTKViewer.h
    VTKRenderer/imstkVTKOpenVRViewer.h
    VTKRenderer/imstkVTKViewer.h
    VTKRenderer/imstkVTKInteractorStyleVR.h)

set(VTK_CPP_FILES
    VTKRenderer/imstkVTKTextStatusManager.cpp
    VTKRenderer/imstkVTKInteractorStyle.cpp
    VTKRenderer/imstkVTKScreenCaptureUtility.cpp
    VTKRenderer/imstkAbstractVTKViewer.cpp
    VTKRenderer/imstkVTKOpenVRViewer.cpp
    VTKRenderer/imstkVTKViewer.cpp
    VTKRenderer/imstkVTKInteractorStyleVR.cpp)

set(VULKAN_H_FILES
    VulkanRenderer/imstkVulkanInteractorStyle.h
    VulkanRenderer/imstkVulkanInteractorStyleFreeCamera.h
    VulkanRenderer/imstkVulkanScreenCaptureUtility.h
    VulkanRenderer/imstkVulkanViewer.h
    VulkanRenderer/imstkVulkanInteractorStyleVR.h)

set(VULKAN_CPP_FILES
    VulkanRenderer/imstkVulkanInteractorStyle.cpp
    VulkanRenderer/imstkVulkanInteractorStyleFreeCamera.cpp
    VulkanRenderer/imstkVulkanScreenCaptureUtility.cpp
    VulkanRenderer/imstkVulkanViewer.cpp
    VulkanRenderer/imstkVulkanInteractorStyleVR.cpp)

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
imstk_add_library(SimulationManager
  H_FILES
    imstkConsoleThread.h
    imstkInteractorStyle.h
    imstkKeyboardSceneControl.h
    imstkMouseSceneControl.h
    imstkSceneManager.h
    imstkScreenCaptureUtility.h
    imstkViewer.h
    ${SIMULATIONMANAGER_H_FILES}
  CPP_FILES
    imstkConsoleThread.cpp
    imstkKeyboardSceneControl.cpp
    imstkMouseSceneControl.cpp
    imstkSceneManager.cpp
    imstkScreenCaptureUtility.cpp
    imstkViewer.cpp
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
