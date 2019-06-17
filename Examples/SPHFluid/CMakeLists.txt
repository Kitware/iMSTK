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

project(Example-SPHFluid)

#-----------------------------------------------------------------------------
# Create executable
#-----------------------------------------------------------------------------
add_executable(${PROJECT_NAME}-BallDrop Main1.cpp Fluid.cpp Solid.cpp Bunny.cpp)
add_executable(${PROJECT_NAME}-HighViscous Main2.cpp Fluid.cpp Solid.cpp Bunny.cpp)
add_executable(${PROJECT_NAME}-BunnyShape Main3.cpp Fluid.cpp Solid.cpp Bunny.cpp)

#-----------------------------------------------------------------------------
# Link libraries to executable
#-----------------------------------------------------------------------------
target_link_libraries(${PROJECT_NAME}-BallDrop SimulationManager)
target_link_libraries(${PROJECT_NAME}-HighViscous SimulationManager)
target_link_libraries(${PROJECT_NAME}-BunnyShape SimulationManager)

#-----------------------------------------------------------------------------
# Add shaders
#-----------------------------------------------------------------------------
include(imstkCopyAndCompileShaders)
CopyAndCompileShaders()

#-----------------------------------------------------------------------------
# Associate external data
#-----------------------------------------------------------------------------
