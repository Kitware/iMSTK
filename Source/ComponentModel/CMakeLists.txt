#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------

include(imstkAddLibrary)
imstk_add_library( ComponentModel
  H_FILES
    imstkComponent.h
    imstkEntity.h
  CPP_FILES
    imstkEntity.cpp
  DEPENDS
    Common
  )

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
if( ${PROJECT_NAME}_BUILD_TESTING )
  add_subdirectory(Testing)
endif()
