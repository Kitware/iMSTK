#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------
include(imstkAddLibrary)
imstk_add_library( CollisionHandling
  DEPENDS
    CollisionDetection
    SceneEntities
    Controllers)


