#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------
include(imstkAddLibrary)
imstk_add_library( Collision
  DEPENDS
    DataStructures
    Geometry
    SceneElements
    DynamicalModels
    SCCD
  )

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
if( ${PROJECT_NAME}_BUILD_TESTING )

  include(imstkAddTest)

  imstk_add_test(Collision)
  imstk_add_data(Collision ${FILE_LIST_COL_TEST})
endif()
