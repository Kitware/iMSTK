#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------

include(imstkAddLibrary)
imstk_add_library( Scene
  H_FILES
    imstkCollisionInteraction.h
    imstkPbdObjectCollision.h
    imstkPbdObjectCutting.h
    imstkPbdObjectGrasping.h
    imstkPbdObjectStitching.h
    imstkPbdRigidObjectCollision.h
    imstkPbdRigidObjectGrasping.h
    imstkRigidObjectCollision.h
    imstkRigidObjectLevelSetCollision.h
    imstkScene.h
    imstkSphObjectCollision.h
  CPP_FILES
    imstkCollisionInteraction.cpp
    imstkPbdObjectCollision.cpp
    imstkPbdObjectCutting.cpp
    imstkPbdObjectGrasping.cpp
    imstkPbdObjectStitching.cpp
    imstkPbdRigidObjectCollision.cpp
    imstkPbdRigidObjectGrasping.cpp
    imstkRigidObjectCollision.cpp
    imstkRigidObjectLevelSetCollision.cpp
    imstkScene.cpp
    imstkSphObjectCollision.cpp
  DEPENDS
    Common
    CollisionDetection
    CollisionHandling
    SceneEntities
    Filtering
  )

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
if( ${PROJECT_NAME}_BUILD_TESTING )
  add_subdirectory( Testing )
  add_subdirectory( VisualTesting )
endif()