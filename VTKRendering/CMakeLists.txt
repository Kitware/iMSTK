
include(${VTK_USE_FILE})

simmedtk_add_library(VTKRendering
  SOURCES
    VTKViewer.cpp
    VTKMeshRenderDelegate.cpp
    VTKPlaneRenderDelegate.cpp
    VTKSceneObjectDeformableRenderDelegate.cpp
    VTKStaticSceneObjectRenderDelegate.cpp
    initVTKRendering.cpp
  PUBLIC_HEADERS
    VTKViewer.h
    initVTKRendering.h
)
set(PRIVATE_LIBS
    PRIVATE
        Core
        Simulators
        Rendering
        Mesh
        Geometry
        )

if(VTK_LIBRARIES)
    target_link_libraries(VTKRendering
    PUBLIC
        ${VTK_LIBRARIES}
    ${PRIVATE_LIBS}
    )
else()
    target_link_libraries(VTKRendering
    PUBLIC
        vtkHybrid
    ${PRIVATE_LIBS}
    )

endif()

if(BUILD_TESTING)
  add_subdirectory(UnitTests)
endif()
