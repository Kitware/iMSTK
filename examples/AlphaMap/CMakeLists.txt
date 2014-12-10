
if(CMAKE_COMPILER_IS_GNUCXX)
  set_source_files_properties(AlphaMappingExample.cpp PROPERTIES COMPILE_FLAGS -std=c++11)
endif()

add_executable(AlphaMap AlphaMappingExample.cpp  AlphaMappingExample.h)
target_link_libraries(AlphaMap sofmis)

