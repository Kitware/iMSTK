#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------
include(imstkAddLibrary)
imstk_add_library( Collision
  DEPENDS
    Datastructures
    SCCD
    Geometry
    SceneElements
    DynamicalModels
  )

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
if( BUILD_TESTING )
  include(imstkAddTest)
  imstk_add_test( Collision )
endif()
