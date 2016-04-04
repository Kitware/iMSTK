#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------
include(imstkAddLibrary)
imstk_add_library( Core
  H_FILES
    imstkMath.h
    imstkColor.h
    imstkModule.h
  CPP_FILES
    imstkColor.cpp
    imstkModule.cpp
  LIBRARIES
    Utilities
  )

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
if( iMSTK_BUILD_TESTING )
  add_subdirectory( Testing )
endif()
