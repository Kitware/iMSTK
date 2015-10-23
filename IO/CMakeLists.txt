simmedtk_add_library(IO
  SOURCES
    initIO.cpp
    IOMesh.cpp
    IOMesh3dsDelegate.cpp
    IOMeshAssimpDelegate.cpp
    IOMeshDelegate.cpp
    IOMeshVegaDelegate.cpp
    IOMeshVTKDelegate.cpp
  PUBLIC_HEADERS
    initIO.h
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
    Assimp::Zlib
    VegaFEM::minivector
    VegaFEM::volumetricMesh
    )

target_compile_options(IO PRIVATE $<$<CXX_COMPILER_ID:GNU>:-Wno-sign-compare -Wno-narrowing>)
if(BUILD_TESTING)
  add_subdirectory(UnitTests)
endif()
