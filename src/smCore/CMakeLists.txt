# include_directories for dependencies actually used here.

set(smCore_sources
  smCore/smConfig.cpp
  smCore/smCoreClass.cpp
  smCore/smDispatcher.cpp
  smCore/smDoubleBuffer.cpp
  smCore/smErrorLog.cpp
  smCore/smEvent.cpp
  smCore/smEventHandler.cpp
  smCore/smGeometry.cpp
  smCore/smModule.cpp
  smCore/smObjectSimulator.cpp
  smCore/smScene.cpp
  smCore/smSceneObject.cpp
  smCore/smScriptingEngine.cpp
  smCore/smSDK.cpp
  smCore/smSimulator.cpp
  smCore/smSpatialGrid.cpp
  smCore/smStaticSceneObject.cpp
  smCore/smTextureManager.cpp)

add_library(smCore OBJECT
  ${smCore_sources})