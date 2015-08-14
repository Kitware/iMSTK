simmedtk_add_library(RenderDelegates
  SOURCES
    AABBRenderDelegate.cpp
    MeshRenderDelegate.cpp
    PlaneRenderDelegate.cpp
    SceneObjectDeformableRenderDelegate.cpp
    StaticSceneObjectRenderDelegate.cpp
#     StylusRenderDelegate.cpp
    SurfaceTreeRenderDelegate.cpp
  PUBLIC_HEADERS
    Config.h
)

target_link_libraries(RenderDelegates
  PRIVATE
    Core
    Rendering
    Mesh
    Simulators
)
