#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------
include(imstkAddLibrary)
imstk_add_library( SimulationManager
  H_FILES
    imstkSimulationManager.h
  CPP_FILES
    imstkSimulationManager.cpp
  LIBRARIES
    Scene
    Rendering
  )
  
#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
if( iMSTK_BUILD_TESTING )
  add_subdirectory( Testing )
endif()
