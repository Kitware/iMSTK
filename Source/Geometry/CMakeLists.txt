#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------
include(imstkAddLibrary)
imstk_add_library( Geometry
  DEPENDS
    Common
    ${VTK_LIBRARIES})

target_compile_options(Geometry PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/bigobj>)

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
if( ${PROJECT_NAME}_BUILD_TESTING )
  include(imstkAddTest)
  imstk_add_test( Geometry )
endif()
