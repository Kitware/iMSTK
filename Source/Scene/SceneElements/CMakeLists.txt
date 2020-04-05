
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
    Common
    Geometry
    DataStructures
    Constraints
    Devices
    DynamicalModels
    ${iMSTK_ODE_DEPENDS}
  )

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
#if( ${PROJECT_NAME}_BUILD_TESTING )
#  add_subdirectory( Testing )
#endif()
