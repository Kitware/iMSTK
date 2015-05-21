
simmedtk_add_library(smUtilities
  SOURCES
    smGLUtils.cpp
    smDisplayText.cpp
    smIOStream.cpp
    smMath.cpp
    smMotionTransformer.cpp
    smTimer.cpp
  PUBLIC_HEADERS
    smGLUtils.h
    smDisplayText.h
    smIOStream.h
)

target_link_libraries(smUtilities
  PRIVATE
    smCore
    smEvent
)
