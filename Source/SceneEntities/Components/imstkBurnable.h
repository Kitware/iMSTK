/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkComponent.h"
#include "imstkMath.h"
#include "imstkVecDataArray.h"
#include "imstkPbdObjectCellRemoval.h"

#pragma once

namespace imstk
{
class PbdConstraint;
class PbdObject;
class PbdObjectCellRemoval;
class TaskNode;

///
/// \class Burnable
///
/// \brief Defines the behaviour to allow a Pbd Object to be burned.  This object creates the state data on the mesh to store both the
/// damage state that controls when a cell gets deleted and state data for visualization. This is currently defined as [0,1] where 0 is
///  no damage and value >= 1 deletes the cell. The visual data is purely for visualization, and can be used as the user desires.
///  The model that changes the state is defined by the burning object.
///
class Burnable : public SceneBehaviour
{
public:
    Burnable(const std::string& name = "BurnableBehaviour");

    ///
    /// \brief Get the PBD object to be torn
    ///@{
    std::shared_ptr<PbdObject> getPbdObject() const { return m_burnableObject; }
    ///@}

    void setTrackOnly(bool val) { m_trackOnly = val; }

    bool getTrackOnly() const { return m_trackOnly; }

    // Check state of mesh and delete cells that are burned away
    // void update(double dt);
    void visualUpdate(const double& dt) override;

    void setUpdateType(PbdObjectCellRemoval::OtherMeshUpdateType updateType)
    {
        m_updateType = updateType;
    }

    PbdObjectCellRemoval::OtherMeshUpdateType getUpdateType() const
    {
        return m_updateType;
    }

    std::shared_ptr<PbdObjectCellRemoval> getCellRemover() const {
        return m_cellRemover;
    }

protected:

    // Allocate state against mesh
    void init() override;

    bool m_trackOnly = false;

    PbdObjectCellRemoval::OtherMeshUpdateType m_updateType = PbdObjectCellRemoval::OtherMeshUpdateType::None;

    std::shared_ptr<PbdObject> m_burnableObject;             ///< Object being burned
    std::shared_ptr<PbdObjectCellRemoval> m_cellRemover;     ///< Cell remover for removing cells

    std::shared_ptr<DataArray<double>> m_burnDamagePtr;
    std::shared_ptr<DataArray<double>> m_burnVisualPtr;
};
} // namespace imstk