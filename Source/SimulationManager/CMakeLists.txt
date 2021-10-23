#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------

file(GLOB H_FILES imstk*.h)
file(GLOB SRC_FILES imstk*.cpp)

set(H_FILES
  imstkConsoleModule.h
  imstkKeyboardSceneControl.h
  imstkMouseSceneControl.h
  imstkSceneManager.h
  imstkSimulationManager.h)

set(SRC_FILES
  imstkConsoleModule.cpp
  imstkKeyboardSceneControl.cpp
  imstkMouseSceneControl.cpp
  imstkSceneManager.cpp
  imstkSimulationManager.cpp)

include(imstkAddLibrary)
imstk_add_library(SimulationManager
  H_FILES
    ${H_FILES}
  CPP_FILES
    ${SRC_FILES}
  DEPENDS
    Scene
    ViewerCore)

# Install VR actions json
file(GLOB VTK_VR_JSON_FILES VTKRenderer/vtk_openvr_*.json)
if (MSVC)
  # Show them in visual studios
  target_sources(SimulationManager PRIVATE ${VTK_VR_JSON_FILES})
  set_source_files_properties(${VTK_VR_JSON_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)
endif()
install(FILES ${VTK_VR_JSON_FILES} DESTINATION bin)

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
if( ${PROJECT_NAME}_BUILD_TESTING )
  add_subdirectory(Testing)
endif()