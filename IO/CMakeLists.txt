find_package(VTK REQUIRED)

include(${VTK_USE_FILE})

simmedtk_add_library(IO
  SOURCES
    IOMesh.cpp
    IOMeshAssimpDelegate.cpp
    IOMeshVegaDelegate.cpp
    IOMeshVTKDelegate.cpp
    IOMesh3dsDelegate.cpp
  PUBLIC_HEADERS
    IOMesh.h
    IOMeshDelegate.h
)

target_link_libraries(IO
  PRIVATE
    Core
    Mesh
  PUBLIC
    ${VTK_LIBRARIES}
    Assimp::Assimp
    VegaFEM::minivector
    VegaFEM::volumetricMesh
    )

target_compile_options(IO PRIVATE $<$<CXX_COMPILER_ID:GNU>:-Wno-sign-compare -Wno-narrowing>)
if(BUILD_TESTING)
  add_subdirectory(UnitTests)
endif()
