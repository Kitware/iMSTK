#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------

include(imstkAddLibrary)
imstk_add_library( Constraints
  H_FILES
    PbdConstraints/imstkPbdAngularConstraint.h
    PbdConstraints/imstkPbdContactConstraint.h
    imstkLinearProjectionConstraint.h
    PbdConstraints/imstkPbdAreaConstraint.h
    PbdConstraints/imstkPbdBaryPointToPointConstraint.h
    PbdConstraints/imstkPbdBendConstraint.h
    PbdConstraints/imstkPbdBody.h
    PbdConstraints/imstkPbdCollisionConstraint.h
    PbdConstraints/imstkPbdConstantDensityConstraint.h
    PbdConstraints/imstkPbdConstraint.h
    PbdConstraints/imstkPbdConstraintContainer.h
    PbdConstraints/imstkPbdDihedralConstraint.h
    PbdConstraints/imstkPbdDistanceConstraint.h
    PbdConstraints/imstkPbdEdgeEdgeCCDConstraint.h
    PbdConstraints/imstkPbdEdgeEdgeConstraint.h
    PbdConstraints/imstkPbdStrainEnergyConstraint.h
    PbdConstraints/imstkPbdStrainEnergyTetConstraint.h
    PbdConstraints/imstkPbdPointEdgeConstraint.h
    PbdConstraints/imstkPbdPointPointConstraint.h
    PbdConstraints/imstkPbdPointTriangleConstraint.h
    PbdConstraints/imstkPbdVolumeConstraint.h
  CPP_FILES
    PbdConstraints/imstkPbdAngularConstraint.cpp
    PbdConstraints/imstkPbdContactConstraint.cpp
    imstkLinearProjectionConstraint.cpp
    PbdConstraints/imstkPbdAreaConstraint.cpp
    PbdConstraints/imstkPbdBaryPointToPointConstraint.cpp
    PbdConstraints/imstkPbdBendConstraint.cpp
    PbdConstraints/imstkPbdCollisionConstraint.cpp
    PbdConstraints/imstkPbdConstantDensityConstraint.cpp
    PbdConstraints/imstkPbdConstraint.cpp
    PbdConstraints/imstkPbdConstraintContainer.cpp
    PbdConstraints/imstkPbdDihedralConstraint.cpp
    PbdConstraints/imstkPbdDistanceConstraint.cpp
    PbdConstraints/imstkPbdEdgeEdgeCCDConstraint.cpp
    PbdConstraints/imstkPbdEdgeEdgeConstraint.cpp
    PbdConstraints/imstkPbdStrainEnergyTetConstraint.cpp
    PbdConstraints/imstkPbdPointEdgeConstraint.cpp
    PbdConstraints/imstkPbdPointPointConstraint.cpp
    PbdConstraints/imstkPbdPointTriangleConstraint.cpp
    PbdConstraints/imstkPbdVolumeConstraint.cpp
  DEPENDS
    CollisionDetection
    Common
    DataStructures
    Geometry
  )

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
if( ${PROJECT_NAME}_BUILD_TESTING )
  add_subdirectory(Testing)
endif()
