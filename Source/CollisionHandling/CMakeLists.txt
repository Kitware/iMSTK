#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------

include(imstkAddLibrary)
imstk_add_library( CollisionHandling
  H_FILES
    imstkBoneDrillingCH.h
    imstkCollisionHandling.h
    imstkLevelSetCH.h
    imstkPbdCollisionHandling.h
    imstkSphCollisionHandling.h
  CPP_FILES
    imstkBoneDrillingCH.cpp
    imstkCollisionHandling.cpp
    imstkLevelSetCH.cpp
    imstkPbdCollisionHandling.cpp
    imstkSphCollisionHandling.cpp
  DEPENDS
    CollisionDetection
    SceneEntities
    Controllers
    )
