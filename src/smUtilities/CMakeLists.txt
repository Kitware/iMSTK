
simmedtk_add_library(smUtilities
  SOURCES
    smGLUtils.cpp
  PUBLIC_HEADERS
    smGLUtils.h
)

target_link_libraries(smUtilities
  PRIVATE
    smCore
)
