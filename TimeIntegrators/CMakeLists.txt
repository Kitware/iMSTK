
set(COMPONENT_MODULE TimeIntegrators)
simmedtk_add_library(${COMPONENT_MODULE}
  SOURCES
    TimeIntegrator.cpp
    ForwardEuler.cpp
    BackwarEuler.cpp

  PUBLIC_HEADERS
    TimeIntegrator.h
    ForwardEuler.h
    BackwarEuler.h
)

target_compile_options(${COMPONENT_MODULE}
  PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wno-old-style-cast -Wno-multichar -Wno-type-limits>)

target_link_libraries(${COMPONENT_MODULE}
  PRIVATE
    Core
    Solvers
)

if(BUILD_TESTING)
  add_subdirectory(UnitTests)
endif()
