#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------
include(imstkAddLibrary)
imstk_add_library( ForceModel
  DEPENDS
    Core
    VegaFEM::massSpringSystem
    VegaFEM::corotationalLinearFEM
    VegaFEM::isotropicHyperelasticFEM
  )

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
if( iMSTK_BUILD_TESTING )
  add_subdirectory( Testing )
endif()
