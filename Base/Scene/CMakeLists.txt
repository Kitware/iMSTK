#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------
include(imstkAddLibrary)
imstk_add_library( Scene
  H_FILES
    imstkScene.h
    imstkSceneObject.h
  CPP_FILES
    imstkScene.cpp
    imstkSceneObject.cpp
  LIBRARIES
    Core
    Geometry
  )

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
if( iMSTK_BUILD_TESTING )
  add_subdirectory( Testing )
endif()
