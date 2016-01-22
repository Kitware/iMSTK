###########################################################################
#
# Copyright (c) Kitware, Inc.
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0.txt
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
###########################################################################

imstk_add_library(Event
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
