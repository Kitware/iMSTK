###########################################################################
#
# This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
# iMSTK is distributed under the Apache License, Version 2.0.
# See accompanying NOTICE for details. 
#
###########################################################################


if (iMSTK_USE_RENDERING_VTK)
  project(Example-PBDFluids)

  #-----------------------------------------------------------------------------
  # Create executable
  #-----------------------------------------------------------------------------
  imstk_add_executable(${PROJECT_NAME} PBDFluidsExample.cpp)

  #-----------------------------------------------------------------------------
  # Add the target to Examples folder
  #-----------------------------------------------------------------------------
  set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Examples/Physics)

  #-----------------------------------------------------------------------------
  # Link libraries to executable
  #-----------------------------------------------------------------------------
  target_link_libraries(${PROJECT_NAME}
    SimulationManager
    ViewerVTK)
endif()