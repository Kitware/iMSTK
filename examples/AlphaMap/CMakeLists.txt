
add_executable(AlphaMap AlphaMappingExample.cpp  AlphaMappingExample.h)
target_link_libraries(AlphaMap
  smCore
  smRendering
  smExternalDevices
  smMesh
  )
