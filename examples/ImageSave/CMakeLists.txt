
add_executable(ImageSave ImageSave.cpp ImageSave.h)
target_link_libraries(ImageSave simmedtk
  ${QT_LIBRARIES})

if(CMAKE_COMPILER_IS_GNUCXX)
  target_compile_options(ImageSave
    PRIVATE
      -std=c++11
  )
endif()
