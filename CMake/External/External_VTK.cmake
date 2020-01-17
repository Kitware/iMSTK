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

set(VTK_GIT_TAG)
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
    -DBUILD_EXAMPLES:BOOL=OFF
    -DBUILD_TESTING:BOOL=OFF
    -DVTK_Group_StandAlone:BOOL=OFF
    -DVTK_Group_Rendering:BOOL=OFF
  )
  set(VTK_GIT_TAG "v8.2.0")
else()
  if (${${PROJECT_NAME}_ENABLE_OPENVR})
    set(VTK_OPENVR "WANT")
  else()
    set(VTK_OPENVR "DONT_WANT")
  endif()
  set(VTK_MODULE_SETTINGS
    -DVTK_MODULE_ENABLE_VTK_RenderingOpenGL2:STRING=YES
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
    -DVTK_BUILD_EXAMPLES:STRING=DONT_WANT
    -DVTK_BUILD_TESTING:STRING=OFF
    -DVTK_GROUP_ENABLE_StandAlone:STRING=DONT_WANT
    -DVTK_GROUP_ENABLE_Rendering:STRING=DONT_WANT
  )
  if(${PROJECT_NAME}_VTK_REPO_SOURCE EQUAL "8.9")
    set(VTK_GIT_TAG "9b6a039f43404053a0653f742148d123f6ada7d6")
  else()
    set(VTK_GIT_TAG "origin/${${PROJECT_NAME}_VTK_REPO_SOURCE}")
  endif()
endif()

#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
include(imstkAddExternalProject)
imstk_add_external_project( VTK
  GIT_REPOSITORY https://gitlab.kitware.com/vtk/vtk.git
  GIT_TAG ${VTK_GIT_TAG}
  CMAKE_ARGS
       ${VTK_MODULE_SETTINGS}
      -DVTK_WRAP_PYTHON:BOOL=OFF
      -DVTK_LEGACY_REMOVE:BOOL=ON
  DEPENDENCIES ${VTK_DEPENDENCIES}
  RELATIVE_INCLUDE_PATH ""
  #VERBOSE
  )
