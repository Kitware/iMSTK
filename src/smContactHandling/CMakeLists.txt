
simmedtk_add_library(smContactHandling
  SOURCES
    smPenaltyContactHandling.cpp
    smPenaltyContactFemToStatic.cpp
  PUBLIC_HEADERS
    smPenaltyContactHandling.h
    smPenaltyContactFemToStatic.h
)

target_link_libraries(smContactHandling
  PUBLIC
    smCore
  PRIVATE
    smEvent
    smCollision
    smSimulators
)

if(BUILD_TESTING)
    add_subdirectory(UnitTests)
endif()
