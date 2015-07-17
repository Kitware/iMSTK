
simmedtk_add_library(smEvent
  SOURCES
    smCameraEvent.cpp
    smHapticEvent.cpp
    smKeyboardEvent.cpp
    smLightMotionEvent.cpp
    smMouseButtonEvent.cpp
    smMouseMoveEvent.cpp
    smObjectClickedEvent.cpp
    smAudioEvent.cpp
  PUBLIC_HEADERS
    smCameraEvent.h
    smHapticEvent.h
    smKeyboardEvent.h
    smLightMotionEvent.h
    smMouseButtonEvent.h
    smObjectClickedEvent.h
)

target_compile_options(smEvent
  PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wno-old-style-cast>)

target_link_libraries(smEvent
  PRIVATE
    smCore
)

if(BUILD_TESTING)
  add_subdirectory(UnitTests)
endif()
