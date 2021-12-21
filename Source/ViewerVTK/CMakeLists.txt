#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------

file(GLOB H_FILES imstk*.h)
file(GLOB CPP_FILES imstk*.cpp)

include(imstkAddLibrary)
imstk_add_library(ViewerVTK
  H_FILES
    ${H_FILES}
  CPP_FILES
    ${CPP_FILES}
  DEPENDS
    ViewerCore
    RenderingVTK)

# Install VR actions json
file(GLOB VTK_VR_JSON_FILES VTKRenderer/vtk_openvr_*.json)
if (MSVC)
  # Show them in visual studios
  target_sources(ViewerVTK PRIVATE ${VTK_VR_JSON_FILES})
  set_source_files_properties(${VTK_VR_JSON_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)
endif()
install(FILES ${VTK_VR_JSON_FILES} DESTINATION bin)

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
# if( ${PROJECT_NAME}_BUILD_TESTING )
#   add_subdirectory(Testing)
# endif()