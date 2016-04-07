#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------
include(imstkAddLibrary)
imstk_add_library( Rendering
  H_FILES
    imstkViewer.h
    imstkRenderer.h
    imstkRenderDelegate.h
    imstkPlaneRenderDelegate.h
    imstkSphereRenderDelegate.h
    imstkCubeRenderDelegate.h
  CPP_FILES
    imstkViewer.cpp
    imstkRenderer.cpp
    imstkRenderDelegate.cpp
    imstkPlaneRenderDelegate.cpp
    imstkSphereRenderDelegate.cpp
    imstkCubeRenderDelegate.cpp
  LIBRARIES
    ${VTK_LIBRARIES}
    Scene
  )

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
if( iMSTK_BUILD_TESTING )
  add_subdirectory( Testing )
endif()
