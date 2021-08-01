#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------
if( ${PROJECT_NAME}_BUILD_TESTING )
include(imstkAddLibrary)
imstk_add_library( Testing
  DEPENDS
    Common
    GTest::gtest
    ${VTK_LIBRARIES}
  )

endif()