#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------
include(imstkAddLibrary)
imstk_add_library( Common
  DEPENDS
    Eigen3::Eigen
    g3log
    tbb
  )

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
if( ${PROJECT_NAME}_BUILD_TESTING )
  include(imstkAddTest)
  imstk_add_test( Common )
endif()
