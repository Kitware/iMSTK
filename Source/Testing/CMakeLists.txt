#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------

if( ${PROJECT_NAME}_BUILD_TESTING )
  include(imstkAddLibrary)
  imstk_add_library( Testing
    CPP_FILES
      imstkTestingMain.cpp
    DEPENDS
      Common
      GTest::gtest
      VTK::CommonSystem
      )
endif()