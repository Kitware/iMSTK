
simmedtk_add_library(ContactHandling
  SOURCES
    PenaltyContactFemToStatic.cpp
    PenaltyContactHandling.cpp
  PUBLIC_HEADERS
    PenaltyContactFemToStatic.h
    PenaltyContactHandling.h
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
