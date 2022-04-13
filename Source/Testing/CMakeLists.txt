#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------

include(imstkAddLibrary)

imstk_add_library( Testing
  H_FILES
    imstkTestingUtils.h
  CPP_FILES
    imstkTestingMain.cpp
  DEPENDS
    Common
    GTest::gtest
    VTK::CommonSystem
  )