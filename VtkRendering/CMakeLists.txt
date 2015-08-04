include(${VTK_USE_FILE})

simmedtk_add_library(VtkRendering
  SOURCES
    VtkViewer.cpp
    VtkMeshRenderDelegate.cpp
  PUBLIC_HEADERS
    VtkViewer.h
)
set(PRIVATE_LIBS
    PRIVATE
        Core)
if(VTK_LIBRARIES)
    target_link_libraries(VtkRendering
    PUBLIC
        ${VTK_LIBRARIES}
    ${PRIVATE_LIBS}
    )
else()
    target_link_libraries(VtkRendering
    PUBLIC
        vtkHybrid
    ${PRIVATE_LIBS}
    )

endif()

if(BUILD_TESTING)
  add_subdirectory(UnitTests)
endif()
