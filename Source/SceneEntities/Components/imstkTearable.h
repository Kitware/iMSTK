/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkComponent.h"
#include "imstkMath.h"

#pragma once

namespace imstk
{
class PbdConstraint;
class PbdObject;
class PbdObjectCellRemoval;
class TaskNode;

///
/// \class Tearable
///
/// \brief Defines the behaviour to allow a mesh to seperate based on strain in a given cell.
/// This class approximates strain using the constraint value in from the PBD solver. This is
/// well defined for line meshes, but the behavior may not be what is expected for surface or
/// tet meshes.
///
///
class Tearable : public SceneBehaviour
{
public:
    Tearable(const std::string& name = "TearablePbdObjectBehavior");

    void init() override;

    ///
    /// \brief Get the PBD object to be torn
    ///@{
    std::shared_ptr<PbdObject> getPbdObject() const { return m_tearableObject; }
    ///@}

    ///
    /// \brief Get/Set the maximum strain. This measure is defined by
    /// (current constraint value / reference value) where the reference value
    /// is not zero.  If the reference value is zero, the constraint value is used.
    ///@{
    double getMaxStrain() const { return m_maxStrain; }
    void setMaxStrain(const double maxStrain) { m_maxStrain = maxStrain; }
///@}

protected:

    // The handle checks the constraint value (strain) of each cell, and if it
    // is greater than the max strain removes the cell.
    void handleTearable();

    void initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink) override;

    std::shared_ptr<PbdObject> m_tearableObject;                   ///< Object being torn
    std::shared_ptr<PbdObjectCellRemoval> m_cellRemover = nullptr; ///< Cell remover for removing cells

    std::shared_ptr<TaskNode> m_tearableHandleNode;

    // Failure (strain) of the PbdObject.  Measured by (current constraint value / reference value) where the
    // reference value is not zero.  If the reference value is zero, the constraint value is used.
    double m_maxStrain = 0.5;
};
} // namespace imstk