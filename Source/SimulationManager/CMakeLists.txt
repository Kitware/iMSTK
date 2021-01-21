#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------

file(GLOB VTK_H_FILES VTKRenderer/imstk*.h)
file(GLOB VTK_CPP_FILES VTKRenderer/imstk*.cpp)

file(GLOB VULKAN_H_FILES VulkanRenderer/imstk*.h)
file(GLOB VULKAN_CPP_FILES VulkanRenderer/imstk*.cpp)

file(GLOB SIMMANAGER_H_FILES imstk*.h)
file(GLOB SIMMANAGER_CPP_FILES imstk*.cpp)

if( NOT iMSTK_USE_Vulkan )
  set(SIMULATIONMANAGER_H_FILES ${SIMMANAGER_H_FILES} ${VTK_H_FILES})
  set(SIMULATIONMANAGER_CPP_FILES ${SIMMANAGER_CPP_FILES} ${VTK_CPP_FILES})
  set(SIMULATIONMANAGER_SUBDIR
    VTKRenderer)
else()
  set(SIMULATIONMANAGER_H_FILES  ${SIMMANAGER_H_FILES} ${VULKAN_H_FILES})
  set(SIMULATIONMANAGER_CPP_FILES ${SIMMANAGER_CPP_FILES} ${VULKAN_CPP_FILES})
  set(SIMULATIONMANAGER_SUBDIR
    VulkanRenderer)
endif()

include(imstkAddLibrary)
imstk_add_library(SimulationManager
  H_FILES
    ${SIMULATIONMANAGER_H_FILES}
  CPP_FILES
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
