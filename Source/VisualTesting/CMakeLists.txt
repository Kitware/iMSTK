#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------

include(imstkAddLibrary)

# Links to a great deal of imstk to provide
# visual testing capabilities
imstk_add_library( VisualTesting
  H_FILES
    imstkVisualTestingUtils.h
  CPP_FILES
    imstkVisualTestingUtils.cpp
  DEPENDS
    Testing
    SimulationManager
    ViewerVTK
  )