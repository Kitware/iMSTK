
simmedtk_add_library(CollisionContext
  SOURCES
    CollisionContext.cpp
  PUBLIC_HEADERS
    CollisionContext.h
)

target_link_libraries(CollisionContext
  PUBLIC
    Core
  PRIVATE
    Event
    Collision
    Simulators
	ContactHandling
)

if(BUILD_TESTING)
  add_subdirectory(UnitTests)
endif()
