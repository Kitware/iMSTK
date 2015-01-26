
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

if(CMAKE_COMPILER_IS_GNUCXX)
  target_compile_options(smTools
    PRIVATE
      -std=c++11
  )
endif()
