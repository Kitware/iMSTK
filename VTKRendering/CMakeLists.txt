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

target_link_libraries(VTKRendering
  PRIVATE
    Core
    Simulators
    Rendering
    Mesh
    Geometry
    Devices
  PUBLIC
    ${VTK_LIBRARIES}
    )

target_compile_options(VTKRendering
  PUBLIC
    $<$<CXX_COMPILER_ID:Clang>:-Wno-inconsistent-missing-override>
)

if(BUILD_TESTING)
  add_subdirectory(UnitTests)
endif()
