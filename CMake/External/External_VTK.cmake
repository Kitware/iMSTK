#-----------------------------------------------------------------------------
# Dependencies
#-----------------------------------------------------------------------------
set(VTK_DEPENDENCIES "")
set(VTK_ENABLE_OPENVR "NO")
if(${${PROJECT_NAME}_ENABLE_VR})
  list(APPEND VTK_DEPENDENCIES "openvr")
  set(VTK_ENABLE_OPENVR "YES")
endif()

#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
include(imstkAddExternalProject)
imstk_add_external_project( VTK
  GIT_REPOSITORY https://gitlab.kitware.com/vtk/vtk.git
  GIT_TAG 9b6a039f43404053a0653f742148d123f6ada7d6 # 8.9  
  CMAKE_ARGS
      -DBUILD_EXAMPLES:BOOL=OFF
      -DBUILD_TESTING:BOOL=OFF
      -DVTK_Group_StandAlone:BOOL=OFF
      -DVTK_Group_Rendering:BOOL=OFF
      -DVTK_MODULE_ENABLE_VTK_RenderingOpenGL2:STRING=YES
      -DVTK_MODULE_ENABLE_VTK_IOXML:STRING=YES
      -DVTK_MODULE_ENABLE_VTK_IOLegacy:STRING=YES
      -DVTK_MODULE_ENABLE_VTK_IOPLY:STRING=YES
      -DVTK_MODULE_ENABLE_VTK_IOGeometry:STRING=YES
      -DVTK_MODULE_ENABLE_VTK_InteractionStyle:STRING=YES
      -DVTK_MODULE_ENABLE_VTK_RenderingAnnotation:STRING=YES
      -DVTK_MODULE_ENABLE_VTK_RenderingOpenVR:STRING=${VTK_ENABLE_OPENVR}
      -DVTK_MODULE_ENABLE_VTK_InteractionWidgets:STRING=YES
      -DVTK_MODULE_ENABLE_VTK_glew:STRING=YES
      -DVTK_MODULE_ENABLE_VTK_RenderingContext2D:STRING=YES
      -DVTK_RENDERING_BACKEND:STRING=OpenGL2
      -DVTK_WRAP_PYTHON:BOOL=OFF
      -DVTK_OPENVR_OBJECT_FACTORY:BOOL=OFF
      -DVTK_LEGACY_REMOVE:BOOL=ON
      -DVTK_BUILD_TESTING:STRING=OFF
  DEPENDENCIES ${VTK_DEPENDENCIES}
  RELATIVE_INCLUDE_PATH ""
  #VERBOSE
  )
