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

imstk_add_library(ContactHandling
  SOURCES
    PenaltyContactFemToStatic.cpp
    PenaltyContactHandling.cpp
  PUBLIC_HEADERS
    PenaltyContactFemToStatic.h
    PenaltyContactHandling.h
)

target_link_libraries(ContactHandling
  PRIVATE
    Core
    Event
    Collision
    Simulators
    Solvers
    SceneModels
)

if(BUILD_TESTING)
    add_subdirectory(UnitTests)
endif()
