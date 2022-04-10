#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------

include(imstkAddLibrary)
imstk_add_library( Animation
  H_FILES
    Particles/imstkRenderParticleEmitter.h
  CPP_FILES
    Particles/imstkRenderParticleEmitter.cpp
  DEPENDS
    Common
    Geometry
    Materials
    SceneEntities
  )

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
#if( ${PROJECT_NAME}_BUILD_TESTING )
#  add_subdirectory( Testing )
#endif()
