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

imstk_add_library(Simulators
  SOURCES
    DefaultSimulator.cpp
    ObjectSimulator.cpp
  PUBLIC_HEADERS
    DefaultSimulator.h
    ObjectSimulator.h
)

target_link_libraries(Simulators
  PRIVATE
    Core
    Mesh
    Event
    Collision
    IO
    Devices
    VirtualTools
    Solvers
    TimeIntegrators
    SceneModels
  PUBLIC
    VegaFEM::configFile
    VegaFEM::vega-getopts
    VegaFEM::performanceCounter
    VegaFEM::volumetricMesh
    VegaFEM::stvk
    VegaFEM::integratorSparse
    VegaFEM::massSpringSystem
    VegaFEM::corotationalLinearFEM
    VegaFEM::elasticForceModel
    VegaFEM::isotropicHyperelasticFEM
    VegaFEM::loadList
    VegaFEM::objMesh
    VegaFEM::volumetricMesh
    VegaFEM::forceModel
    )

target_include_directories(Simulators
  PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    )
