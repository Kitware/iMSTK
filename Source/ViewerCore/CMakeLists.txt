#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------

set(H_FILES
  imstkViewer.h)
set(CPP_FILES
  imstkViewer.cpp)

include(imstkAddLibrary)
imstk_add_library(ViewerCore
  H_FILES
    ${H_FILES}
  CPP_FILES
    ${CPP_FILES}
  EXCLUDE_FILES
    ${ExclusionFiles}
  DEPENDS
    RenderingCore)

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
# if( ${PROJECT_NAME}_BUILD_TESTING )
#   add_subdirectory(Testing)
# endif()