#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------
include(imstkAddLibrary)
imstk_add_library( Scene
  H_FILES
    imstkScene.h
    imstkCollisionGraph.h
  CPP_FILES
    imstkScene.cpp
    imstkCollisionGraph.cpp
  DEPENDS
    Common
    Collision
    SceneElements
    Controllers
    DynamicalModels
  )

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
#if( ${PROJECT_NAME}_BUILD_TESTING )
#  add_subdirectory( Testing )
#endif()