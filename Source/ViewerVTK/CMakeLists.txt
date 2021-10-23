#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------

file(GLOB H_FILES imstk*.h VTKViewer/imstk*.h)
file(GLOB CPP_FILES imstk*.cpp VTKViewer/imstk*.cpp)

list(APPEND ExclusionFiles "")
# Don't build with X11 if on windows or (on linux and building headless)
if(WIN32 OR (UNIX AND iMSTK_USE_VTK_OSMESA))
  # We use our own vtkXRenderWindowInteractor as there is a bug in vtk9's
  list(APPEND ExclusionFiles
    VTKViewer/imstkVtkXRenderWindowInteractor2.h
    VTKViewer/imstkVtkXRenderWindowInteractor2.cpp)
endif()

set(SIMULATIONMANAGER_H_FILES ${SIMMANAGER_H_FILES} ${VTK_H_FILES})
set(SIMULATIONMANAGER_CPP_FILES ${SIMMANAGER_CPP_FILES} ${VTK_CPP_FILES})

include(imstkAddLibrary)
imstk_add_library(Viewer
  H_FILES
    ${H_FILES}
  CPP_FILES
    ${CPP_FILES}
  SUBDIR_LIST
    VTKViewer
  EXCLUDE_FILES
    ${ExclusionFiles}
  DEPENDS
    RenderingVTK)

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
# if( ${PROJECT_NAME}_BUILD_TESTING )
#   add_subdirectory(Testing)
# endif()