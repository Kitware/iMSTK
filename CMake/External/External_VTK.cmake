#-----------------------------------------------------------------------------
# Dependencies
#-----------------------------------------------------------------------------
set(VTK_DEPENDENCIES "")
set(VTK_ENABLE_OPENVR "NO")
if(${${PROJECT_NAME}_ENABLE_VR})
  list(APPEND VTK_DEPENDENCIES "openvr")
  set(VTK_ENABLE_OPENVR "YES")
endif()


set(${PROJECT_NAME}_VTK_REPO_SOURCE "8.2" CACHE STRING "Select VTK Source Branch/Tag")
set(VTK_SOURCES "8.2;8.9;master;release;nightly-master" CACHE INTERNAL "List of available VTK branch,tags to get")
set_property(CACHE ${PROJECT_NAME}_VTK_REPO_SOURCE PROPERTY STRINGS ${VTK_SOURCES})


if(${PROJECT_NAME}_VTK_REPO_SOURCE EQUAL "8.2")
  set(VTK_MODULE_SETTINGS
    -DModule_vtkRenderingOpenGL2:BOOL=ON
    -DModule_vtkIOXML:BOOL=ON
    -DModule_vtkIOLegacy:BOOL=ON
    -DModule_vtkIOPLY:BOOL=ON
    -DModule_vtkIOGeometry:BOOL=ON
    -DModule_vtkInteractionStyle:BOOL=ON
    -DModule_vtkRenderingAnnotation:BOOL=ON
    -DModule_vtkRenderingOpenVR:BOOL=${${PROJECT_NAME}_ENABLE_VR}
    -DModule_vtkInteractionWidgets:BOOL=ON
    -DModule_vtkglew:BOOL=ON
    -DModule_vtkRenderingContext2D:BOOL=ON
    -DModule_vtkRenderingVolumeOpenGL2:BOOL=ON
    -DBUILD_EXAMPLES:BOOL=OFF
    -DBUILD_TESTING:BOOL=OFF
    -DVTK_Group_StandAlone:BOOL=OFF
    -DVTK_Group_Rendering:BOOL=OFF
  )
  set(${PROJECT_NAME}_VTK_SOURCE URL https://gitlab.kitware.com/vtk/vtk/-/archive/v8.2.0/vtk-v8.2.0.tar.gz)
  set(${PROJECT_NAME}_VTK_HASH URL_HASH MD5=4115fb396f99466fe444472f412118cd)
else()
  if (${${PROJECT_NAME}_ENABLE_OPENVR})
    set(VTK_OPENVR "WANT")
  else()
    set(VTK_OPENVR "DONT_WANT")
  endif()
  set(VTK_MODULE_SETTINGS
    -DVTK_MODULE_ENABLE_VTK_RenderingOpenGL2:STRING=YES
    -DVTK_MODULE_ENABLE_VTK_IOExport:STRING=YES
    -DVTK_MODULE_ENABLE_VTK_IOImport:STRING=YES
    -DVTK_MODULE_ENABLE_VTK_IOParallel:STRING=YES
    -DVTK_MODULE_ENABLE_VTK_IOParallelXML:STRING=YES
    -DVTK_MODULE_ENABLE_VTK_IOXML:STRING=YES
    -DVTK_MODULE_ENABLE_VTK_IOLegacy:STRING=YES
    -DVTK_MODULE_ENABLE_VTK_IOPLY:STRING=YES
    -DVTK_MODULE_ENABLE_VTK_IOGeometry:STRING=YES
    -DVTK_MODULE_ENABLE_VTK_InteractionStyle:STRING=YES
    -DVTK_MODULE_ENABLE_VTK_RenderingAnnotation:STRING=YES
    -DVTK_MODULE_ENABLE_VTK_RenderingOpenVR:STRING=${VTK_OPENVR}
    -DVTK_MODULE_ENABLE_VTK_InteractionWidgets:STRING=YES
    -DVTK_MODULE_ENABLE_VTK_glew:STRING=YES
    -DVTK_MODULE_ENABLE_VTK_RenderingContext2D:STRING=YES
    -DVTK_MODULE_ENABLE_VTK_RenderingVolumeOpenGL2:STRING=YES
    -DVTK_BUILD_EXAMPLES:STRING=DONT_WANT
    -DVTK_BUILD_TESTING:STRING=OFF
    -DVTK_GROUP_ENABLE_StandAlone:STRING=DONT_WANT
    -DVTK_GROUP_ENABLE_Rendering:STRING=DONT_WANT
  )
  set(${PROJECT_NAME}_VTK_SOURCE GIT_REPOSITORY https://gitlab.kitware.com/vtk/vtk.git)
 
  # TODO Update to a zip download when there is a new version after 8.2
  if(${PROJECT_NAME}_VTK_REPO_SOURCE EQUAL "8.9")
    set(${PROJECT_NAME}_VTK_SOURCE GIT_REPOSITORY https://gitlab.kitware.com/vtk/vtk.git)
    set(${PROJECT_NAME}_VTK_HASH GIT_TAG 9b6a039f43404053a0653f742148d123f6ada7d6)
  else()
    set(${PROJECT_NAME}_VTK_SOURCE GIT_REPOSITORY https://gitlab.kitware.com/vtk/vtk.git)
    set(${PROJECT_NAME}_VTK_HASH GIT_TAG origin/${${PROJECT_NAME}_VTK_REPO_SOURCE})
  endif()
endif()

#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
include(imstkAddExternalProject)
imstk_add_external_project( VTK
  ${${PROJECT_NAME}_VTK_SOURCE}
  ${${PROJECT_NAME}_VTK_HASH}
  CMAKE_ARGS
       ${VTK_MODULE_SETTINGS}
      -DVTK_WRAP_PYTHON:BOOL=OFF
      -DVTK_LEGACY_REMOVE:BOOL=ON
  DEPENDENCIES ${VTK_DEPENDENCIES}
  RELATIVE_INCLUDE_PATH ""
  #VERBOSE
)
if(NOT USE_SYSTEM_VTK)
  set(VTK_DIR ${CMAKE_INSTALL_PREFIX}/lib/cmake/vtk-8.2)
  message(STATUS "VTK_DIR : ${VTK_DIR}")
endif()
