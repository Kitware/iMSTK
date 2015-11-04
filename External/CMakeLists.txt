include(${VTK_USE_FILE})
simmedtk_add_library(External
  SOURCES
    FrameBufferObject.cpp
    RenderBuffer.cpp
  PUBLIC_HEADERS
    FrameBufferObject.h
    moller.h
    moller2.h
    RenderBuffer.h
    tree.hh
)

target_compile_options(External
  PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wno-old-style-cast -Wno-multichar -Wno-type-limits>)

target_link_libraries(External
  PRIVATE
    Core
)
