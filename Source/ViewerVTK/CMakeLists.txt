#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------

file(GLOB H_FILES imstk*.h)
file(GLOB CPP_FILES imstk*.cpp)

list(APPEND ExclusionFiles "")
# Don't build with X11 if on windows or (on linux and building headless)
if(WIN32 OR (UNIX AND iMSTK_USE_VTK_OSMESA))
  # We use our own vtkXRenderWindowInteractor as there is a bug in vtk9's
  list(APPEND ExclusionFiles
    imstkVtkXRenderWindowInteractor2.h
    imstkVtkXRenderWindowInteractor2.cpp)
endif()

include(imstkAddLibrary)
imstk_add_library(ViewerVTK
  H_FILES
    ${H_FILES}
  CPP_FILES
    ${CPP_FILES}
  SUBDIR_LIST
    VTKViewer
  EXCLUDE_FILES
    ${ExclusionFiles}
  DEPENDS
    ViewerCore
    RenderingVTK)

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
# if( ${PROJECT_NAME}_BUILD_TESTING )
#   add_subdirectory(Testing)
# endif()