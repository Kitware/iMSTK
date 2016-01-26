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

set(APP AVMNidus)

include(${VTK_USE_FILE})

add_executable(${APP} main.cpp)
target_link_libraries(${APP}
  Core
  Geometry
  Rendering
  Rendering
  Simulators
  Mesh
  Collision
  ContactHandling
  Event
  IO
  Solvers
  SceneModels
  SimulationManager
  )

set_target_properties(${APP}
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)

set(SOURCE_PATH ${PROJECT_SOURCE_DIR}/Testing)
set(BINARY_PATH ${PROJECT_BINARY_DIR}/Testing/ExamplesData/Nidus)
configure_file(${PROJECT_SOURCE_DIR}/Testing/path.in ${CMAKE_CURRENT_BINARY_DIR}/Config.paths @ONLY)
