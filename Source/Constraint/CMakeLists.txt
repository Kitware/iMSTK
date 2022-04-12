#-----------------------------------------------------------------------------
# Create target
#-----------------------------------------------------------------------------

include(imstkAddLibrary)
imstk_add_library( Constraints
  H_FILES
    imstkLinearProjectionConstraint.h
    MultiModelConstraints/imstkPbdRigidBaryPointToPointConstraint.h
    PbdConstraints/imstkPbdAreaConstraint.h
    PbdConstraints/imstkPbdBaryPointToPointConstraint.h
    PbdConstraints/imstkPbdBendConstraint.h
    PbdConstraints/imstkPbdCollisionConstraint.h
    PbdConstraints/imstkPbdConstantDensityConstraint.h
    PbdConstraints/imstkPbdConstraint.h
    PbdConstraints/imstkPbdConstraintContainer.h
    PbdConstraints/imstkPbdDihedralConstraint.h
    PbdConstraints/imstkPbdDistanceConstraint.h
    PbdConstraints/imstkPbdEdgeEdgeConstraint.h
    PbdConstraints/imstkPbdFemConstraint.h
    PbdConstraints/imstkPbdFemTetConstraint.h
    PbdConstraints/imstkPbdPointEdgeConstraint.h
    PbdConstraints/imstkPbdPointPointConstraint.h
    PbdConstraints/imstkPbdPointTriangleConstraint.h
    PbdConstraints/imstkPbdVolumeConstraint.h
    RigidBodyConstraints/imstkRbdConstraint.h
    RigidBodyConstraints/imstkRbdContactConstraint.h
    RigidBodyConstraints/imstkRbdDistanceConstraint.h
    RigidBodyConstraints/imstkRbdFrictionConstraint.h
  CPP_FILES
    imstkLinearProjectionConstraint.cpp
    MultiModelConstraints/imstkPbdRigidBaryPointToPointConstraint.cpp
    PbdConstraints/imstkPbdAreaConstraint.cpp
    PbdConstraints/imstkPbdBaryPointToPointConstraint.cpp
    PbdConstraints/imstkPbdBendConstraint.cpp
    PbdConstraints/imstkPbdCollisionConstraint.cpp
    PbdConstraints/imstkPbdConstantDensityConstraint.cpp
    PbdConstraints/imstkPbdConstraint.cpp
    PbdConstraints/imstkPbdConstraintContainer.cpp
    PbdConstraints/imstkPbdDihedralConstraint.cpp
    PbdConstraints/imstkPbdDistanceConstraint.cpp
    PbdConstraints/imstkPbdEdgeEdgeConstraint.cpp
    PbdConstraints/imstkPbdFemConstraint.cpp
    PbdConstraints/imstkPbdFemTetConstraint.cpp
    PbdConstraints/imstkPbdPointEdgeConstraint.cpp
    PbdConstraints/imstkPbdPointPointConstraint.cpp
    PbdConstraints/imstkPbdPointTriangleConstraint.cpp
    PbdConstraints/imstkPbdVolumeConstraint.cpp
    RigidBodyConstraints/imstkRbdConstraint.cpp
    RigidBodyConstraints/imstkRbdContactConstraint.cpp
    RigidBodyConstraints/imstkRbdDistanceConstraint.cpp
    RigidBodyConstraints/imstkRbdFrictionConstraint.cpp
  DEPENDS
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