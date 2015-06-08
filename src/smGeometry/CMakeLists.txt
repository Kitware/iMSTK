
simmedtk_add_library(smGeometry
  SOURCES
    smMeshModel.cpp
    smPlaneModel.cpp
  PUBLIC_HEADERS
    ${CMAKE_SOURCE_DIR}/include/smGeometry/smMeshModel.h
)

target_link_libraries(smGeometry
  PRIVATE
    smCore
    smRendering
)

if(BUILD_TESTING)
  add_subdirectory(UnitTests)
endif()
