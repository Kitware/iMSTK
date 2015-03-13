
add_executable(ImageSave ImageSave.cpp ImageSave.h)
target_link_libraries(ImageSave
  smCore
  smRendering
  smMesh
)
