
set(MODULE_NAME Assembler)
simmedtk_add_library(${MODULE_NAME}
  SOURCES
    Assembler.cpp
  PUBLIC_HEADERS
    Assembler.h
)

target_link_libraries(Assembler
  PUBLIC
    Core
  PRIVATE
    Event
	ContactHandling
	Simulators
)
