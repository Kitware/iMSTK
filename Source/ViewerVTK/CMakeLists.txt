#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------

include(imstkAddLibrary)
imstk_add_library(ViewerVTK
  H_FILES
    imstkAbstractVTKViewer.h
    imstkScreenCaptureUtility.h
    imstkVTKInteractorStyle.h
    imstkVTKInteractorStyleVR.h
    imstkVtkOpenVRRenderWindowInteractorImstk.h
    imstkVTKOpenVRViewer.h
    imstkVTKScreenCaptureUtility.h
    imstkVTKViewer.h
  CPP_FILES
    imstkAbstractVTKViewer.cpp
    imstkScreenCaptureUtility.cpp
    imstkVTKInteractorStyle.cpp
    imstkVTKInteractorStyleVR.cpp
    imstkVtkOpenVRRenderWindowInteractorImstk.cpp
    imstkVTKOpenVRViewer.cpp
    imstkVTKScreenCaptureUtility.cpp
    imstkVTKViewer.cpp
  DEPENDS
    ViewerCore
    RenderingVTK
    VTK::InteractionStyle
  )

# Install VR actions json
file(GLOB VTK_VR_JSON_FILES vtk_openvr_*.json)
if (MSVC)
  # Show them in visual studios
  target_sources(ViewerVTK PRIVATE ${VTK_VR_JSON_FILES})
  set_source_files_properties(${VTK_VR_JSON_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)
endif()
install(FILES ${VTK_VR_JSON_FILES} DESTINATION bin)

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
if( ${PROJECT_NAME}_BUILD_TESTING )
  add_subdirectory(Testing)
endif()