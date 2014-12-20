
if(CMAKE_COMPILER_IS_GNUCXX)
  set_source_files_properties(main.cpp vegaFemExample.cpp PROPERTIES COMPILE_FLAGS -std=c++11)
endif()

add_executable(vegaFemExample main.cpp  vegaFemExample.cpp vegaFemExample.h)
target_link_libraries(vegaFemExample sofmis)

