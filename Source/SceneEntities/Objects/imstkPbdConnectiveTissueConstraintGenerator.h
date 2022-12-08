/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkPbdConstraintFunctor.h"

#include <unordered_map>

namespace imstk
{
class PbdObject;
class PbdConstraintContainer;
class PbdBaryPointToPointConstraint;
class PbdModel;
class ProximitySurfaceSelector;

///
/// \class PbdConectiveTissueConstraintGenerator
///
/// \brief This class takes in three pbd objects. The first two represent the bodies
/// that the user wants to connect and the third is the line mesh representation of
/// the connective tissue that is generated using the ConnectiveStrandGenerator filter.
/// The class then generates a set of constraints that connect the ends of the connective
/// strands to the associated organ. The process assumes that the tips of the strands are
/// coincident with the surface of the two pbd objects being connected.
///

class PbdConnectiveTissueConstraintGenerator : public PbdBodyConstraintFunctor
{
public:
    PbdConnectiveTissueConstraintGenerator() = default;
    ~PbdConnectiveTissueConstraintGenerator() override = default;

    ///
    /// \brief Sets PBD object that represents the connective tissue.
    /// This must use a line mesh for the physics mesh
    ///
    void setConnectiveStrandObj(std::shared_ptr<PbdObject> connectiveStrandObj) { m_connectiveStrandObj = connectiveStrandObj; }

    ///
    /// \brief Creates distance constraints for the connective strands
    /// using the default m_distStiffness value. To modify the stiffness,
    ///  call setDistStiffness before calling generateDistanceConstraints
    ///
    void generateDistanceConstraints();

    ///
    /// \brief Sets PBD object to connect to connective strands
    /// assumes the surface mesh of ObjA is coincident with the
    /// end points of one side of the strands
    ///
    void setConnectedObjA(std::shared_ptr<PbdObject> objA) { m_objA = objA; }

    ///
    /// \brief Sets PBD object to connect to connective straints
    /// assumes the surface mesh of ObjB is coincident with the
    /// end points of one side of the strands
    ///
    void setConnectedObjB(std::shared_ptr<PbdObject> objB) { m_objB = objB; }

    ///
    /// \brief Appends a set of constraint to the container given a geometry & body
    ///
    void operator()(PbdConstraintContainer& constraints) override;

    ///
    /// \brief Get/Set the stiffness for the distance constraint.
    ///@{
    void setDistStiffness(double stiffness) { m_distStiffness = stiffness; }
    double getDistStiffness() { return m_distStiffness; }
    ///@}

    ///
    /// \brief Get/Set Tolerance for checking coincidence of surface to line mesh
    ///@{
    void setTolerance(double tolerance) { m_tolerance = tolerance; }
    double getTolerance() { return m_tolerance; }
///@}

protected:

    ///
    /// \brief Used to generate connecting constraints when the body being attached uses a tet mesh as
    /// the physics mesh.
    ///
    void connectLineToTetMesh(
        std::shared_ptr<PbdObject> pbdObj,
        PbdConstraintContainer&    constraints);

    ///
    /// \brief Used to generate connecting constraints when the body being attached uses a surface mesh as
    /// the physics mesh.
    ///
    void connectLineToSurfMesh(
        std::shared_ptr<PbdObject> pbdObj,
        PbdConstraintContainer&    constraints);

    std::shared_ptr<PbdObject> m_connectiveStrandObj = nullptr; ///< Connective tissue that is made
    std::shared_ptr<PbdObject> m_objA = nullptr;                ///< Organ being connected
    std::shared_ptr<PbdObject> m_objB = nullptr;                ///< Organ being connected

    double m_distStiffness = 50.0;                              ///< Stiffness used for distance constraints
    double m_tolerance     = 0.00001;                           ///< Tolerance for checking coincidence of surface to line mesh
};

///
/// \fn addConnectiveTissueConstraints
///
/// \brief Helper function for creating constraints on the connective tissue
///
std::shared_ptr<PbdObject> addConnectiveTissueConstraints(
    std::shared_ptr<LineMesh>  connectiveLineMesh,
    std::shared_ptr<PbdObject> objA,
    std::shared_ptr<PbdObject> objB,
    std::shared_ptr<PbdModel>  model);

///
/// \fn makeConnectiveTissue
///
/// \brief This function takes in the PbdObjects that represent the organs to be connected, the
/// associated PbdModel, and the parameters used to control the generation of the connective tissue
/// PbdObject that is returned.
///
/// \param objA PbdObject that represents an organ to be connected with connective tissue
/// \param objB PbdObject that represents an organ to be connected with connective tissue
/// \param maxDist maximum distance of faces between objA and objB that are connected
/// \param strandsPerFace number of strands generated per face, if this has a fractional part
///        there is a chance (equal to the fractional part) of the face having one more strand
/// \param segmentsPerStrand  number of segments each strand is made of
/// \param selector the selector type used to generate the faces, currently only ProximitySurfaceSelector
///
std::shared_ptr<PbdObject> makeConnectiveTissue(
    std::shared_ptr<PbdObject>                objA,
    std::shared_ptr<PbdObject>                objB,
    std::shared_ptr<PbdModel>                 model,
    double                                    maxDist = 0.0,
    double                                    strandsPerFace    = 1,
    int                                       segmentsPerStrand = 3,
    std::shared_ptr<ProximitySurfaceSelector> selector = nullptr);
} // namespace imstk