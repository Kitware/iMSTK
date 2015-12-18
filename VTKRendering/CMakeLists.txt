include(${VTK_USE_FILE})

simmedtk_add_library(VTKRendering
  SOURCES
    VTKViewer.cpp
    VTKMeshRenderDelegate.cpp
    VTKPlaneRenderDelegate.cpp
    VTKSceneObjectDeformableRenderDelegate.cpp
    VTKStaticSceneObjectRenderDelegate.cpp
    InitVTKRendering.cpp
  PUBLIC_HEADERS
    VTKViewer.h
    InitVTKRendering.h
)
set(PRIVATE_LIBS
    PRIVATE
        Core
        Simulators
        Rendering
        Mesh
        Geometry
        Devices
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

target_compile_options(VTKRendering
  PUBLIC
    $<$<CXX_COMPILER_ID:Clang>:-Wno-inconsistent-missing-override>
)

if(BUILD_TESTING)
  add_subdirectory(UnitTests)
endif()
