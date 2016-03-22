#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------
include(imstkAddLibrary)
imstk_add_library( Utilities
  H_FILES
    imstkLogUtility.h
  LIBRARIES
   g3log
  )

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
if( iMSTK_BUILD_TESTING )
  add_subdirectory( Testing )
endif()
