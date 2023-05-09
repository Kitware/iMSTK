###########################################################################
#
# This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
# iMSTK is distributed under the Apache License, Version 2.0.
# See accompanying NOTICE for details. 
#
###########################################################################

# Haptic only example
if(iMSTK_USE_HAPTICS AND iMSTK_USE_RENDERING_VTK)
    project(Example-PBDDynamicSuture)

    #-----------------------------------------------------------------------------
    # Create executable
    #-----------------------------------------------------------------------------
    imstk_add_executable(${PROJECT_NAME}
        pbdDynamicSutureExample.cpp

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
        ViewerVTK
        Needle
        )
endif()