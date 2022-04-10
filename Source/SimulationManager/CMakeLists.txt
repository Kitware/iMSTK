#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------

set(H_FILES
  imstkConsoleModule.h
  imstkKeyboardSceneControl.h
  imstkMouseSceneControl.h
  imstkSceneManager.h
  imstkSimulationManager.h
  )

set(SRC_FILES
  imstkConsoleModule.cpp
  imstkKeyboardSceneControl.cpp
  imstkMouseSceneControl.cpp
  imstkSceneManager.cpp
  imstkSimulationManager.cpp
  )

include(imstkAddLibrary)
imstk_add_library(SimulationManager
  H_FILES
    ${H_FILES}
  CPP_FILES
    ${SRC_FILES}
  DEPENDS
    Scene
    ViewerCore
  )

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
if( ${PROJECT_NAME}_BUILD_TESTING )
  add_subdirectory(Testing)
endif()