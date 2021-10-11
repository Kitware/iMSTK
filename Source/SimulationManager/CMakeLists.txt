#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------

file(GLOB VTK_H_FILES VTKRenderer/imstk*.h)
file(GLOB VTK_CPP_FILES VTKRenderer/imstk*.cpp)

list(APPEND ExclusionFiles "")
# Don't build with X11 if on windows or (on linux and building headless)
if(WIN32 OR (UNIX AND iMSTK_USE_VTK_OSMESA))
  list(APPEND ExclusionFiles
    VTKRenderer/imstkVtkXRenderWindowInteractor2.h
    VTKRenderer/imstkVtkXRenderWindowInteractor2.cpp)
endif()

file(GLOB SIMMANAGER_H_FILES imstk*.h)
file(GLOB SIMMANAGER_CPP_FILES imstk*.cpp)

set(SIMULATIONMANAGER_H_FILES ${SIMMANAGER_H_FILES} ${VTK_H_FILES})
set(SIMULATIONMANAGER_CPP_FILES ${SIMMANAGER_CPP_FILES} ${VTK_CPP_FILES})
set(SIMULATIONMANAGER_SUBDIR
  VTKRenderer)

include(imstkAddLibrary)
imstk_add_library(SimulationManager
  H_FILES
    ${SIMULATIONMANAGER_H_FILES}
  CPP_FILES
    ${SIMULATIONMANAGER_CPP_FILES}
  SUBDIR_LIST
    ${SIMULATIONMANAGER_SUBDIR}
  EXCLUDE_FILES
    ${ExclusionFiles}
  DEPENDS
    Scene
    Rendering
  )

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
if( ${PROJECT_NAME}_BUILD_TESTING )
  add_subdirectory(Testing)
endif()