#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------

set(H_FILES
  imstkConsoleModule.h
  imstkFpsTxtCounter.h
  imstkKeyboardSceneControl.h
  imstkMouseSceneControl.h
  imstkPerformanceGraph.h
  imstkSceneControlText.h
  imstkSceneManager.h
  imstkSimulationManager.h
  imstkSimulationUtils.h
  )

set(SRC_FILES
  imstkConsoleModule.cpp
  imstkFpsTxtCounter.cpp
  imstkKeyboardSceneControl.cpp
  imstkMouseSceneControl.cpp
  imstkPerformanceGraph.cpp
  imstkSceneControlText.cpp
  imstkSceneManager.cpp
  imstkSimulationManager.cpp
  imstkSimulationUtils.cpp
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