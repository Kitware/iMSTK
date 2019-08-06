
set(iMSTK_ODE_DEPENDS)
if( iMSTK_USE_ODE)
    # add ode if requested
    set(iMSTK_ODE_DEPENDS libodedouble)
endif()

#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------
include(imstkAddLibrary)
imstk_add_library( SceneElements
  DEPENDS
    Core
    Geometry
    Constraints
    Devices
    TimeIntegrators
    DynamicalModels
    #Solvers
    ${iMSTK_ODE_DEPENDS}
  )

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
if( iMSTK_BUILD_TESTING )
  add_subdirectory( Testing )
endif()
