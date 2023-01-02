###########################################################################
#
# This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
# iMSTK is distributed under the Apache License, Version 2.0.
# See accompanying NOTICE for details. 
#
###########################################################################

if (iMSTK_USE_RENDERING_VTK)
  project(Example-StraightNeedle)

  #-----------------------------------------------------------------------------
  # Create executable
  #-----------------------------------------------------------------------------
  imstk_add_executable(${PROJECT_NAME}
    EmbeddingConstraint.cpp
    EmbeddingConstraint.h
    NeedleEmbedder.cpp
    NeedleEmbedder.h
    NeedleInteraction.cpp
    NeedleInteraction.h
    NeedlePbdCH.h
    NeedleRigidBodyCH.h
    NeedleSurfaceInteraction.h
    PBDTissueSurfaceNeedleContactExample.cpp
    PBDTissueVolumeNeedleContactExample.cpp
    RbdAngularLockingConstraint.h
    RbdAxesLockingConstraint.h
  )

  #-----------------------------------------------------------------------------
  # Add the target to Examples folder
  #-----------------------------------------------------------------------------
  set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Examples/Physics)

  #-----------------------------------------------------------------------------
  # Link libraries to executable
  #-----------------------------------------------------------------------------
  target_link_libraries(${PROJECT_NAME}
    SimulationManager
    ViewerVTK
    Needle
    )
endif()
