
if(CMAKE_COMPILER_IS_GNUCXX)
  set_source_files_properties(ImageSave.cpp PROPERTIES COMPILE_FLAGS -std=c++11)
endif()

add_executable(ImageSave ImageSave.cpp ImageSave.h)
target_link_libraries(ImageSave sofmis)

