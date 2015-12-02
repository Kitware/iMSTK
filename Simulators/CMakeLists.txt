
simmedtk_add_library(Simulators
  SOURCES
    DefaultSimulator.cpp
#     MyStylus.cpp
#     StylusObject.cpp
#     ToolSimulator.cpp
    VegaObjectConfig.cpp
    VegaFemSceneObject.cpp
    VegaFemSimulator.cpp
	SceneObjectDeformable.cpp
  PUBLIC_HEADERS
    DefaultSimulator.h
#     MyStylus.h
#     StylusObject.h
#     ToolSimulator.h
    VegaObjectConfig.h
    VegaFemSceneObject.h
    VegaFemSimulator.h
    SceneObjectDeformable.h
)

target_link_libraries(Simulators
  PRIVATE
    Core
    Mesh
    Event
    Collision
    IO
    Devices
    VirtualTools
  PUBLIC
    VegaFEM::configFile
    VegaFEM::vega-getopts
    VegaFEM::performanceCounter
    VegaFEM::volumetricMesh
    VegaFEM::stvk
    VegaFEM::integratorSparse
    VegaFEM::massSpringSystem
    VegaFEM::corotationalLinearFEM
    VegaFEM::elasticForceModel
    VegaFEM::isotropicHyperelasticFEM
    VegaFEM::loadList
    VegaFEM::objMesh
    VegaFEM::volumetricMesh
    )

target_include_directories(Simulators
  PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    )
