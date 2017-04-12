#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------
include(imstkAddLibrary)
imstk_add_library( Collision
  DEPENDS
    SCCD
    Geometry
    SceneElements
  )

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
if( BUILD_TESTING )
  include(imstkAddTest)
  imstk_add_test( Collision )
endif()
