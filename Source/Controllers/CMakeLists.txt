#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------

include(imstkAddLibrary)
imstk_add_library( Controllers
  DEPENDS
	Devices
    SceneEntities
  )

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
if( ${PROJECT_NAME}_BUILD_TESTING )
  add_subdirectory( Testing )
endif()
