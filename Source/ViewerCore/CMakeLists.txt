#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------

include(imstkAddLibrary)
imstk_add_library(ViewerCore
  H_FILES
    imstkViewer.h
  CPP_FILES
    imstkViewer.cpp
  EXCLUDE_FILES
    ${ExclusionFiles}
  DEPENDS
    RenderingCore
  )

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
# if( ${PROJECT_NAME}_BUILD_TESTING )
#   add_subdirectory(Testing)
# endif()