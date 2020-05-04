#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------
include(imstkAddLibrary)
imstk_add_library( CollisionDetection
  DEPENDS
    Common
    DataStructures
    Geometry
    MeshIO #TODO: remove this dependency
    SCCD    
  )

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
if( ${PROJECT_NAME}_BUILD_TESTING )

  include(imstkAddTest)

  imstk_add_test(CollisionDetection)
  imstk_add_data(CollisionDetection ${FILE_LIST_COL_TEST})
endif()
