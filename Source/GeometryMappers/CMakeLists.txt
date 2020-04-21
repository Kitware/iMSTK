#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------
include(imstkAddLibrary)
imstk_add_library( GeometryMappers
  DEPENDS
	Geometry
  )

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
#if( ${PROJECT_NAME}_BUILD_TESTING )
#  include(imstkAddTest)
#  imstk_add_test( GeometryMappers )
#endif()
