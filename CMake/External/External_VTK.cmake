#-----------------------------------------------------------------------------
# Dependencies
#-----------------------------------------------------------------------------
set(VTK_DEPENDENCIES "")
if(${${PROJECT_NAME}_ENABLE_VR})
  list(APPEND VTK_DEPENDENCIES "openvr")
endif(${${PROJECT_NAME}_ENABLE_VR})

#-----------------------------------------------------------------------------
# Add External Project
#-----------------------------------------------------------------------------
if(${iMSTK_WITH_RENDERING})
  set(rendering_backend "OpenGL2")
else()
  set(rendering_backend "None")
endif()

include(imstkAddExternalProject)
imstk_add_external_project( VTK
  GIT_REPOSITORY https://gitlab.kitware.com/vtk/vtk.git
  GIT_TAG v8.2.0
  INSTALL_COMMAND ${SKIP_STEP_COMMAND}
  CMAKE_ARGS
      -DBUILD_EXAMPLES:BOOL=OFF
      -DBUILD_TESTING:BOOL=OFF
      -DVTK_Group_StandAlone:BOOL=OFF
      -DVTK_Group_Rendering:BOOL=${iMSTK_WITH_RENDERING}
      -DModule_vtkRenderingOpenGL2:BOOL=${iMSTK_WITH_RENDERING}
      -DModule_vtkIOXML:BOOL=ON
      -DModule_vtkIOLegacy:BOOL=ON
      -DModule_vtkIOPLY:BOOL=ON
      -DModule_vtkIOGeometry:BOOL=ON
      -DModule_vtkInteractionStyle:BOOL=ON
      -DModule_vtkRenderingAnnotation:BOOL=${iMSTK_WITH_RENDERING}
      -DModule_vtkRenderingOpenVR:BOOL=${${PROJECT_NAME}_ENABLE_VR}
      -DModule_vtkInteractionWidgets:BOOL=${iMSTK_WITH_RENDERING}
      -DModule_vtkglew:BOOL=${iMSTK_WITH_RENDERING}
      -DModule_vtkRenderingContext2D:BOOL=${iMSTK_WITH_RENDERING}
      -DModule_vtkFiltersCore:BOOL=ON
      -DModule_vtkRenderingCore:BOOL=ON
      -DModule_vtkImagingCore:BOOL=ON
      -DVTK_RENDERING_BACKEND:STRING=${rendering_backend}
      -DVTK_WRAP_PYTHON:BOOL=OFF
      -DVTK_OPENVR_OBJECT_FACTORY:BOOL=OFF
      -DVTK_LEGACY_REMOVE:BOOL=ON
  DEPENDENCIES ${VTK_DEPENDENCIES}
  RELATIVE_INCLUDE_PATH ""
  #VERBOSE
  )
