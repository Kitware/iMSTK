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

if(iMSTK_USE_OpenHaptics AND iMSTK_USE_RENDERING_VTK)
    project(Example-PBDDynamicSuture)

    #-----------------------------------------------------------------------------
    # Create executable
    #-----------------------------------------------------------------------------
    imstk_add_executable(${PROJECT_NAME}
        pbdDynamicSutureExample.cpp
        NeedleObject.h
        NeedleObject.cpp
        NeedleInteraction.h
        NeedleInteraction.cpp
        NeedlePbdCH.h
        NeedlePbdCH.cpp
        SurfaceInsertionConstraint.h
        SurfaceInsertionConstraint.cpp
        ThreadInsertionConstraint.h
        ThreadInsertionConstraint.cpp
    )



    #-----------------------------------------------------------------------------
    # Add the target to Examples folder
    #-----------------------------------------------------------------------------
    SET_TARGET_PROPERTIES (${PROJECT_NAME} PROPERTIES FOLDER Examples/PBD)

    #-----------------------------------------------------------------------------
    # Link libraries to executable
    #-----------------------------------------------------------------------------
    target_link_libraries(${PROJECT_NAME}
        SimulationManager
        ViewerVTK)
endif()