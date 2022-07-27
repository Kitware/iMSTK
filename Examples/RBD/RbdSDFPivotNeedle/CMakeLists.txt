###########################################################################
#
# This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
# iMSTK is distributed under the Apache License, Version 2.0.
# See accompanying NOTICE for details. 
#
###########################################################################


if (iMSTK_USE_RENDERING_VTK)
  project(Example-RbdSDFPivotNeedle)

  #-----------------------------------------------------------------------------
  # Create executable
  #-----------------------------------------------------------------------------
  imstk_add_executable(${PROJECT_NAME} RbdSDFPivotNeedleExample.cpp
    NeedleObject.h
    NeedleInteraction.h
    NeedleInteraction.cpp
    NeedleRigidBodyCH.h
    NeedleRigidBodyCH.cpp
    RbdLineToPointTranslationConstraint.h
    RbdLineToPointRotationConstraint.h
    NeedleRigidBodyCH.h)

  #-----------------------------------------------------------------------------
  # Add the target to Examples folder
  #-----------------------------------------------------------------------------
  set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Examples/RBD)

  #-----------------------------------------------------------------------------
  # Link libraries to executable
  #-----------------------------------------------------------------------------
  target_link_libraries(${PROJECT_NAME}
    SimulationManager
    Filtering
    ViewerVTK)
endif()