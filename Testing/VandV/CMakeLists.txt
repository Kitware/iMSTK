
set(Rendering)
if (iMSTK_USE_RENDERING_VTK)
  set(Rendering ViewerVTK)
endif()

set(_files
  Main.cpp
  ProgrammableScene.h
  ProgrammableScene.cpp
  ProgrammableSceneExec.cpp
  ProgrammableSceneExec.h
  Verification/CSVUtils.cpp
  Verification/CSVUtils.h
  Verification/EntityToVTK.cpp
  Verification/EntityToVTK.h
  Scenes/BendingBeamScene.cpp
  Scenes/BendingBeamScene.h
  Scenes/CapsuleToCube.cpp
  Scenes/CapsuleToCube.h
  Scenes/ChainDropScene.cpp
  Scenes/ChainDropScene.h
  Scenes/LoadableMeshScene.cpp
  Scenes/LoadableMeshScene.h
  Scenes/SpringMassDamperScene.cpp
  Scenes/SpringMassDamperScene.h
  Scenes/UnitCubeStrainScene.cpp
  Scenes/UnitCubeStrainScene.h
  Scenes/Studies/CMBBE2023.cpp
)

imstk_add_executable(VandV ${_files})

target_include_directories(VandV 
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(VandV
  SimulationManager
  VTK::ViewsContext2D
  ${Rendering})

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${_files})
