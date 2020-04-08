#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------
include(imstkAddLibrary)      
imstk_add_library(apiUtilities
  DEPENDS
    Common
    SimulationManager
    Scene
  )
  
#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
#if( ${PROJECT_NAME}_BUILD_TESTING )
#  add_subdirectory( Testing )
#endif()