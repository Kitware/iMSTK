#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------

set(H_FILES
  imstkArcNeedle.h
  imstkNeedle.h
  imstkNeedleInteraction.h
  imstkNeedlePbdCH.h
  imstkPuncture.h
  imstkPuncturable.h
  imstkStraightNeedle.h
  imstkSurfaceInsertionConstraint.h
  imstkThreadInsertionConstraint.h
  )

set(SRC_FILES
  imstkNeedle.cpp
  imstkNeedleInteraction.cpp
  imstkNeedlePbdCH.cpp
  imstkPuncturable.cpp
  imstkStraightNeedle.cpp
  imstkSurfaceInsertionConstraint.cpp
  imstkThreadInsertionConstraint.cpp
  )

include(imstkAddLibrary)
imstk_add_library(Needle
  H_FILES
    ${H_FILES}
  CPP_FILES
    ${SRC_FILES}
  DEPENDS
    ComponentModel
    Geometry
    DynamicalModels
    CollisionHandling
    Constraints
    Scene
  )

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
# if( ${PROJECT_NAME}_BUILD_TESTING )
#   add_subdirectory(Testing)
# endif()