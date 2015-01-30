
add_executable(AlphaMap AlphaMappingExample.cpp  AlphaMappingExample.h)
target_link_libraries(AlphaMap simmedtk)

if(CMAKE_COMPILER_IS_GNUCXX)
  target_compile_options(AlphaMap
    PRIVATE
      -std=c++11
  )
endif()
