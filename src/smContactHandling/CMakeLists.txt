
simmedtk_add_library(smContactHandling
  SOURCES
    smContactHandling.cpp
    smPenaltyContactHandling.cpp
  PUBLIC_HEADERS
    ${CMAKE_SOURCE_DIR}/include/smContactHandling/smPenaltyContactHandling.h
    ${CMAKE_SOURCE_DIR}/include/smContactHandling/smContactHandling.h
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
