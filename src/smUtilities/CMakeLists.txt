
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

if(CMAKE_COMPILER_IS_GNUCXX)
  target_compile_options(smUtilities
    PRIVATE
      -std=c++11
  )
endif()
