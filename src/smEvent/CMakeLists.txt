
simmedtk_add_library(smEvent
  SOURCES
    smEvent.cpp
    smCameraEvent.cpp
    smEventHandler.cpp
    smHapticEvent.cpp
    smKeyboardEvent.cpp
    smLightMotionEvent.cpp
    smMouseButtonEvent.cpp
    smMouseMoveEvent.cpp
    smObjectClickedEvent.cpp
    smAudioEvent.cpp
  PUBLIC_HEADERS
    ${CMAKE_SOURCE_DIR}/include/smEvent/smEvent.h
    ${CMAKE_SOURCE_DIR}/include/smEvent/smCameraEvent.h
    ${CMAKE_SOURCE_DIR}/include/smEvent/smEventHandler.h
    ${CMAKE_SOURCE_DIR}/include/smEvent/smHapticEvent.h
    ${CMAKE_SOURCE_DIR}/include/smEvent/smKeyboardEvent.h
    ${CMAKE_SOURCE_DIR}/include/smEvent/smLightMotionEvent.h
    ${CMAKE_SOURCE_DIR}/include/smEvent/smMouseButtonEvent.h
    ${CMAKE_SOURCE_DIR}/include/smEvent/smObjectClickedEvent.h
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
