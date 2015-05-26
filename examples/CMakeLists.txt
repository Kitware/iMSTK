
add_subdirectory(common)

if(SIMMEDTK_USE_PHANTOM_OMNI)
  add_subdirectory(AlphaMap)
endif(SIMMEDTK_USE_PHANTOM_OMNI)

if(SimMedTK_USE_OCULUS)
  add_subdirectory(renderCubeOculus)
  add_subdirectory(renderCubeToTextureOculus)
endif(SimMedTK_USE_OCULUS)

add_subdirectory(AudioExample)
add_subdirectory(renderCube)
add_subdirectory(renderCubeToTexture)
# add_subdirectory(vegaFem)
# add_subdirectory(MultipleObjects)
add_subdirectory(CollisionDetectionSpatialHashing)
add_subdirectory(CollisionDetectionBVH)
#add_subdirectory(demoExample)
