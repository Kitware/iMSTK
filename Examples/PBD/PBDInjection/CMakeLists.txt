###########################################################################
#
# This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
# iMSTK is distributed under the Apache License, Version 2.0.
# See accompanying NOTICE for details. 
#
###########################################################################

message(STATUS "PbdInjection has been disabled, awaiting refactoring wrt Entity Framework")
# HS 2023-sep-05 Disabled needs refactoring wrt Entity Framework
# There is an issue where we're calling `shared_from_this` in the constructor 
# chain
# if (iMSTK_USE_RENDERING_VTK)
#   project(Example-PBDInjection)
# 
#   #-----------------------------------------------------------------------------
#   # Create executable
#   #-----------------------------------------------------------------------------
#   imstk_add_executable(${PROJECT_NAME}
#     PBDInjectExample.cpp
#     InflatableObject.h
#     InflatableObject.cpp
#     imstkPbdInflatableVolumeConstraint.h
#     imstkPbdInflatableDistanceConstraint.h
#     )
# 
#   #-----------------------------------------------------------------------------
#   # Add the target to Examples folder
#   #-----------------------------------------------------------------------------
#   set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Examples/PBD)
# 
#   #-----------------------------------------------------------------------------
#   # Link libraries to executable
#   #-----------------------------------------------------------------------------
#   target_link_libraries(${PROJECT_NAME}
#     SimulationManager
#     ViewerVTK)
# endif()