#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------

include(imstkAddLibrary)
imstk_add_library( FilteringCore
  H_FILES
    imstkGeometryAlgorithm.h
  CPP_FILES
    imstkGeometryAlgorithm.cpp
  DEPENDS
    Geometry
    DataStructures
    )

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
if( ${PROJECT_NAME}_BUILD_TESTING )
  add_subdirectory(Testing)
endif()