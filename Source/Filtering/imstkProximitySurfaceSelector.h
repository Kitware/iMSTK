/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkGeometryAlgorithm.h"
#include "imstkDataArray.h"

namespace imstk
{
class PointSet;
class SurfaceMesh;

///
/// \class ProximitySurfaceSelector
///
/// \brief This filter takes in two surface meshes and generates
/// two surface meshes that are subsets of the two input meshes.
/// The new surface meshes are made of faces of the two meshes
/// that fall within a distance (m_proximity) of each other.
///
class ProximitySurfaceSelector : public GeometryAlgorithm
{
public:
    ProximitySurfaceSelector();
    ~ProximitySurfaceSelector() override = default;

    void setInputMeshes(
        std::shared_ptr<SurfaceMesh> inputMeshA,
        std::shared_ptr<SurfaceMesh> inputMeshB);

    std::shared_ptr<SurfaceMesh> getOutputMeshA() const;
    std::shared_ptr<SurfaceMesh> getOutputMeshB() const;

    ///
    /// \brief Get/Set the proximity. Any faces within this distance between
    /// the two meshes are added to the output surface meshes.
    ///@{
    void setProximity(double prox) { m_proximity = prox; }
    double getProximity() { return m_proximity; }
///@}

protected:
    void requestUpdate() override;
    double m_proximity = 0.0;
};
} // namespace imstk