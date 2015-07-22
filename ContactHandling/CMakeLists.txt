
simmedtk_add_library(ContactHandling
  SOURCES
    PenaltyContactHandling.cpp
    PenaltyContactFemToStatic.cpp
  PUBLIC_HEADERS
    PenaltyContactHandling.h
    PenaltyContactFemToStatic.h
)

target_link_libraries(ContactHandling
  PUBLIC
    Core
  PRIVATE
    Event
    Collision
    Simulators
)

if(BUILD_TESTING)
    add_subdirectory(UnitTests)
endif()
