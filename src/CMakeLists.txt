# TODO: Make a CMakeLists.txt in each subdirectory (see smCore for an example
# and below (NEWLIB) for an example for how to use it).

set(sources_sofmis
./smCollision/smSpatialHash.cpp
./smCore/smConfig.cpp
./smCore/smCoreClass.cpp
./smCore/smDispatcher.cpp
./smCore/smDoubleBuffer.cpp
./smCore/smErrorLog.cpp
./smCore/smEvent.cpp
./smCore/smEventHandler.cpp
./smCore/smGeometry.cpp
./smCore/smModule.cpp
./smCore/smObjectSimulator.cpp
./smCore/smScene.cpp
./smCore/smSceneObject.cpp
./smCore/smScriptingEngine.cpp
./smCore/smSDK.cpp
./smCore/smSimulator.cpp
./smCore/smSpatialGrid.cpp
./smCore/smStaticSceneObject.cpp
./smCore/smTextureManager.cpp
./smExternal/framebufferObject.cpp
./smExternal/renderbuffer.cpp
./smMesh/smImportExport.cpp
./smMesh/smLattice.cpp
./smMesh/smLatticeType.cpp
./smMesh/smMesh.cpp
./smMesh/smSurfaceMesh.cpp
./smMesh/smVolumeMesh.cpp
./smRenderEffects/smEffectSim.cpp
./smRenderEffects/SmokeEffect.cpp
./smRendering/smConfigRendering.cpp
./smRendering/smFrameBuffer.cpp
./smRendering/smLight.cpp
./smRendering/smRenderer.cpp
./smRendering/smVBO.cpp
./smRendering/smVAO.cpp
./smRendering/smViewer.cpp
./smShader/SceneTextureShader.cpp
./smShader/smShader.cpp
./smSimulators/smFemSceneObject.cpp
./smSimulators/smMyStylus.cpp
./smSimulators/smPBDObjectSimulator.cpp
./smSimulators/smPBDSceneObject.cpp
./smSimulators/smPhysXVolumeMesh.cpp
./smSimulators/smStylusObject.cpp
./smSimulators/smToolSimulator.cpp
./smUtilities/smDisplayText.cpp
./smUtilities/smGLUtils.cpp
./smUtilities/smIOStream.cpp
./smUtilities/smVec3.cpp
)

if(SIMMEDTK_USE_PHANTOM_OMNI)
  set(sources_sofmis ${sources_sofmis}
  ./smExternalDevices/smDeviceInterface.cpp
  ./smExternalDevices/smHapticInterface.cpp
  ./smExternalDevices/smPhantomInterface.cpp)
endif()

set(SIMMEDTK_EXTERNAL_DEVICES_SOURCES
  ./smExternalDevices/smADUInterface.cpp
  ./smExternalDevices/smATC3DGInterface.cpp
  ./smExternalDevices/smNIUSB6008Interface.cpp
)


if(CMAKE_COMPILER_IS_GNUCXX)
  set_source_files_properties(${sources_sofmis} PROPERTIES COMPILE_FLAGS -std=c++11)
endif()

add_library(sofmis
  ${sources_sofmis})

# NEWLIB
#add_library(sofmis
#  dummy_source.cpp # Needed to get the language for the sofmis library. It can be empty.
#  $<TARGET_OBJECTS:smCore> # For each library, add a line like this.
#)

# TODO: There are a lot of missing libraries here; look at the available
# variables in the cmake-gui.
target_link_libraries(sofmis
  ${QGLVIEWER_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_THREAD_LIBRARY}
  ${QT_LIBRARIES}
  ${GLEW_LIBRARY}
  ${GLUT_LIBRARY}
  ${OPENGL_LIBRARIES}
  ${OPENCV_LIBRARIES}
  ${ADU_LIBRARY}
  ${ASSIMP_LIBRARY}
  ${ATC_LIBRARY}
  ${AUDIERE_LIBRARY}
  ${GLUT_glut_LIBRARY}
  ${ILUT_LIBRARIES}
  ${ILU_LIBRARIES}
  ${IL_LIBRARIES}
  ${NIDAQ_mx_LIBRARY}
  ${DevIL_LIBRARY}
  ${OPENHAPTICS_hd_LIBRARY}
  ${OPENHAPTICS_hdu_LIBRARY}
)

if(BUILD_EXTERNAL_DEVICES)
add_library(externalDevices
  ${SIMMEDTK_EXTERNAL_DEVICES_SOURCES})
target_link_libraries(externalDevices
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_THREAD_LIBRARY}
  ${QT_LIBRARIES}
  ${GLEW_LIBRARY}
  ${GLUT_LIBRARY}
  ${OPENGL_LIBRARIES}
  ${OPENCV_LIBRARIES}
  ${ADU_LIBRARY}
  ${ASSIMP_LIBRARY}
  ${ATC_LIBRARY}
  ${AUDIERE_LIBRARY}
  ${GLUT_glut_LIBRARY}
  ${ILUT_LIBRARIES}
  ${ILU_LIBRARIES}
  ${IL_LIBRARIES}
  ${NIDAQ_mx_LIBRARY}
  ${DevIL_LIBRARY}
  ${OPENHAPTICS_hd_LIBRARY}
  ${OPENHAPTICS_hdu_LIBRARY})
endif()
