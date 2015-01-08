
add_executable(vegaFemExample main.cpp  vegaFemExample.cpp vegaFemExample.h)
target_link_libraries(vegaFemExample simmedtk
  ${QT_LIBRARIES})

target_include_directories(vegaFemExample
  PUBLIC
    ${VegaFEM_INCLUDE_DIRS}
)

if(CMAKE_COMPILER_IS_GNUCXX)
  target_compile_options(vegaFemExample
    PRIVATE
      -std=c++11
  )
endif()
