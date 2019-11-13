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

project(Example-Viewer)

#-----------------------------------------------------------------------------
# Create executable
#-----------------------------------------------------------------------------
add_executable(${PROJECT_NAME} ViewerExample.cpp)
vtk_module_autoinit(TARGETS ${PROJECT_NAME} MODULES ${VTK_LIBRARIES})

#-----------------------------------------------------------------------------
# Add the target to Examples folder
#-----------------------------------------------------------------------------
SET_TARGET_PROPERTIES (${PROJECT_NAME} PROPERTIES FOLDER Examples)

#-----------------------------------------------------------------------------
# Link libraries to executable
#-----------------------------------------------------------------------------
target_link_libraries(${PROJECT_NAME} SimulationManager apiUtilities)

#-----------------------------------------------------------------------------
# Set MSVC working directory to the install/bin directory
#-----------------------------------------------------------------------------
if(MSVC) # Configure running executable out of MSVC
  set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${iMSTK_INSTALL_BIN_DIR}")
endif()
