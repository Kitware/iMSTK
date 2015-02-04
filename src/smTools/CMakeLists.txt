
simmedtk_add_library(smTools
  SOURCES
    curvedGrasper.cpp
  PUBLIC_HEADERS
    curvedGrasper.h
)

target_link_libraries(smTools
  PRIVATE
    smCore
)
