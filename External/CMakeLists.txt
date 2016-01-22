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

imstk_add_library(External
  SOURCES
    FrameBufferObject/FrameBufferObject.cpp
    FrameBufferObject/RenderBuffer.cpp
  PUBLIC_HEADERS
    FrameBufferObject/FrameBufferObject.h
    FrameBufferObject/RenderBuffer.h
    Moller/moller2.h
    Moller/moller.h
    Tree/tree.hh
)

target_compile_options(External
  PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wno-old-style-cast -Wno-multichar -Wno-type-limits>)

target_link_libraries(External
  PRIVATE
    Core
)
