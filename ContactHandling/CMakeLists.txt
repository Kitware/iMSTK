simmedtk_add_library(ContactHandling
  SOURCES
    PenaltyContactFemToStatic.cpp
    PenaltyContactHandling.cpp
  PUBLIC_HEADERS
    PenaltyContactFemToStatic.h
    PenaltyContactHandling.h
)

target_link_libraries(ContactHandling
  PRIVATE
    Core
    Event
    Collision
    Simulators
    Solvers
    SceneModels
)

if(BUILD_TESTING)
    add_subdirectory(UnitTests)
endif()
