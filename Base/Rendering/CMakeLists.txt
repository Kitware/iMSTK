#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------
include(imstkAddLibrary)
imstk_add_library( Rendering
  H_FILES
    imstkViewer.h
  CPP_FILES
    imstkViewer.cpp
  LIBRARIES
    ${VTK_LIBRARIES}
    Core
    Scene
  )

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
if( iMSTK_BUILD_TESTING )
  add_subdirectory( Testing )
endif()
