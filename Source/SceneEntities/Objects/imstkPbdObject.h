/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkDynamicObject.h"
#include "imstkMacros.h"
#include "imstkPbdConstraint.h"

namespace imstk
{
class PbdModel;
class PointSet;

///
/// \class PbdObject
///
/// \brief Base class for scene objects that move and/or deform under position
/// based dynamics formulation, implements the PbdModel and PbdSolver
///
class PbdObject : public DynamicObject
{
public:
    PbdObject(const std::string& name = "PbdObject") : DynamicObject(name) { }
    ~PbdObject() override = default;

    IMSTK_TYPE_NAME(PbdObject)

    ///
    /// \biref Get the Pbd model of the object
    ///
    std::shared_ptr<PbdModel> getPbdModel();

    ///
    /// \brief Returns body in the model.
    ///
    std::shared_ptr<PbdBody> getPbdBody()
    {
        if (m_pbdBody == nullptr)
        {
            LOG(FATAL) << "Set the PbdModel on the PbdObject before trying to acquire the body";
        }
        return m_pbdBody;
    }

    ///
    /// \brief Sets the model, and creates the body within the model
    ///
    void setDynamicalModel(std::shared_ptr<AbstractDynamicalModel> dynaModel) override;

    ///
    /// \brief Update physics geometry, overrided to set transform should the
    /// PbdObject be a rigid body
    ///
    void updatePhysicsGeometry() override;

    ///
    /// \brief Sets the PbdBody representing this object given its geometry
    ///
    void setBodyFromGeometry();

    ///
    /// \brief Initialize the Pbd scene object
    ///
    bool initialize() override;

    void computeCellConstraintMap();

    const std::vector<std::shared_ptr<PbdConstraint>>& getCellConstraints(int cellId)
    {
        return m_pbdBody->m_cellConstraintMap[cellId];
    }

protected:
    ///
    /// \brief Creates a deformable PbdBody from Geometry
    ///
    void setDeformBodyFromGeometry(PbdBody& body, std::shared_ptr<PointSet> geom);

    ///
    /// \brief Creates a rigid PbdBody from values
    ///
    void setRigidBody(PbdBody& body);

protected:
    std::shared_ptr<PbdModel> m_pbdModel = nullptr; ///< Pbd mathematical model
    std::shared_ptr<PbdBody>  m_pbdBody  = nullptr; ///< Handle to this object in the model/system
};
} // namespace imstk