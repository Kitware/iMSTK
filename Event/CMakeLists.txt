simmedtk_add_library(Event
  SOURCES
    AudioEvent.cpp
    CameraEvent.cpp
    HapticEvent.cpp
    KeyboardEvent.cpp
    LightMotionEvent.cpp
    MouseButtonEvent.cpp
    MouseMoveEvent.cpp
    ObjectClickedEvent.cpp
  PUBLIC_HEADERS
    AudioEvent.h
    CameraEvent.h
    HapticEvent.h
    Key.h
    KeyboardEvent.h
    KeySFMLInterface.h
    LightMotionEvent.h
    MouseButtonEvent.h
    MouseMoveEvent.h
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
