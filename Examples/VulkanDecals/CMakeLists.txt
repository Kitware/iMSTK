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
if(iMSTK_USE_Vulkan)

    project(Example-VulkanDecals)

    #-----------------------------------------------------------------------------
    # Create executable
    #-----------------------------------------------------------------------------
    add_executable(${PROJECT_NAME} VulkanDecalsExample.cpp)

    #-----------------------------------------------------------------------------
    # Link libraries to executable
    #-----------------------------------------------------------------------------
    target_link_libraries(${PROJECT_NAME} SimulationManager)
        
    #-----------------------------------------------------------------------------
    # Add shaders
    #-----------------------------------------------------------------------------
    include(imstkCopyAndCompileShaders)
    CopyAndCompileShaders()
      
    #-----------------------------------------------------------------------------
    # Associate external data
    #-----------------------------------------------------------------------------
    list(APPEND FILE_LIST
        decals/,REGEX:.*)
    imstk_add_data(${PROJECT_NAME} ${FILE_LIST})

endif()