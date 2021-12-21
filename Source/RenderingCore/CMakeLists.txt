#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------

include(imstkAddLibrary)
imstk_add_library(RenderingCore
  H_FILES
    imstkRenderer.h
    imstkTextureManager.h
    imstkTextureDelegate.h
  CPP_FILES
    imstkRenderer.cpp
  DEPENDS
    Scene)

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
# if( ${PROJECT_NAME}_BUILD_TESTING )
#   add_subdirectory(Testing)
# endif()