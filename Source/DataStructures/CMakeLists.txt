#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------
include(imstkAddLibrary)
imstk_add_library( Datastructures
  DEPENDS
    Core
  )

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
if( BUILD_TESTING )
  include(imstkAddTest)
  imstk_add_test( Datastructures )
endif()
