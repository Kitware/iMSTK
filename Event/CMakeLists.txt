
simmedtk_add_library(Event
  SOURCES
    CameraEvent.cpp
    HapticEvent.cpp
    KeyboardEvent.cpp
    LightMotionEvent.cpp
    MouseButtonEvent.cpp
    MouseMoveEvent.cpp
    ObjectClickedEvent.cpp
    AudioEvent.cpp
  PUBLIC_HEADERS
    CameraEvent.h
    HapticEvent.h
    KeyboardEvent.h
    LightMotionEvent.h
    MouseButtonEvent.h
    ObjectClickedEvent.h
)

target_compile_options(Event
  PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wno-old-style-cast -Wno-multichar -Wno-type-limits>)

target_link_libraries(Event
  PRIVATE
    Core
)

if(BUILD_TESTING)
  add_subdirectory(UnitTests)
endif()
