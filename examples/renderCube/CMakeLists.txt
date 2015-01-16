
if(CMAKE_COMPILER_IS_GNUCXX)
  set_source_files_properties(main.cpp RenderExample.cpp PROPERTIES COMPILE_FLAGS -std=c++11)
endif()

add_executable(renderCube main.cpp  RenderExample.cpp RenderExample.h)
target_link_libraries(renderCube glfw ${GLFW_LIBRARIES})
target_link_libraries(renderCube sofmis)
