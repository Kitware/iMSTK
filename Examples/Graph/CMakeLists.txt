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

project(Example-Graph)

#-----------------------------------------------------------------------------
# Create executable
#-----------------------------------------------------------------------------
add_executable(${PROJECT_NAME} Graph.cpp)

#-----------------------------------------------------------------------------
# Link libraries to executable
#-----------------------------------------------------------------------------
if(UNIX)
  target_link_libraries(${PROJECT_NAME}
  SimulationManager)     
else()
  target_link_libraries(${PROJECT_NAME}
  SimulationManager) 
endif()
  
#-----------------------------------------------------------------------------
# Associate external data
#-----------------------------------------------------------------------------
list(APPEND FILE_LIST
    asianDragon/,REGEX:.*)

imstk_add_data(${PROJECT_NAME} ${FILE_LIST})