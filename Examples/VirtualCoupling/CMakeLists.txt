###########################################################################
#
# This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
# iMSTK is distributed under the Apache License, Version 2.0.
# See accompanying NOTICE for details. 
#
###########################################################################


if(iMSTK_USE_OpenHaptics)
  project(Example-VirtualCoupling)

  #-----------------------------------------------------------------------------
  # Create executable
  #-----------------------------------------------------------------------------
  imstk_add_executable(${PROJECT_NAME} VirtualCouplingExample.cpp)

  #-----------------------------------------------------------------------------
  # Add the target to Examples folder
  #-----------------------------------------------------------------------------
  set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Examples/Haptics)

  #-----------------------------------------------------------------------------
  # Link libraries to executable
  #-----------------------------------------------------------------------------
  if (iMSTK_USE_RENDERING_VTK)
    target_link_libraries(${PROJECT_NAME}
      SimulationManager
      ViewerVTK)

    target_compile_definitions(${PROJECT_NAME} PUBLIC -DiMSTK_USE_RENDERING_VTK)
  else()
    target_link_libraries(${PROJECT_NAME}
      SimulationManager)
  endif()
endif()