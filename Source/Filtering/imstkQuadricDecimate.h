/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkGeometryAlgorithm.h"

namespace imstk
{
class SurfaceMesh;

///
/// \class CleanMesh
///
/// \brief This filter reduces a SurfaceMesh
///
class QuadricDecimate : public GeometryAlgorithm
{
public:
    QuadricDecimate();
    ~QuadricDecimate() override = default;

public:
    ///
    /// \brief Required input, port 0
    ///
    void setInputMesh(std::shared_ptr<SurfaceMesh> inputMesh);

    ///
    /// \brief Get/Set whether to preserve volume or not, default on
    ///@{
    imstkSetMacro(VolumePreserving, bool);
    imstkGetMacro(VolumePreserving, bool);
    ///@}

    ///
    /// \brief Set the target reduction ratio
    ///@{
    imstkSetMacro(TargetReduction, double);
    imstkGetMacro(TargetReduction, double);
///@}

protected:
    void requestUpdate() override;

private:
    bool   m_VolumePreserving;
    double m_TargetReduction;
};
} // namespace imstk