#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------

include(imstkAddLibrary)
imstk_add_library(Materials
  H_FILES
    imstkRenderMaterial.h
    imstkTexture.h
  CPP_FILES
    imstkRenderMaterial.cpp
    imstkTexture.cpp
  DEPENDS
    Common)

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
# if( ${PROJECT_NAME}_BUILD_TESTING )
#   add_subdirectory(Testing)
# endif()