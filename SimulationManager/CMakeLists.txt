
simmedtk_add_library(SimulationManager
  SOURCES
    SDK.cpp
    Simulator.cpp
  PUBLIC_HEADERS
    SDK.h
    Simulator.h
    Synchronization.h
    Scheduler.h
)

target_link_libraries(SimulationManager
  PRIVATE
    Core
    )

if(BUILD_TESTING)
#   add_subdirectory(UnitTests)
endif()
