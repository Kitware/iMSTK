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
/// \class SurfaceMeshSmoothen
///
/// \brief This filter smoothes the input SurfaceMesh currently only laplacian
/// smoothing is provided
///
class SurfaceMeshSmoothen : public GeometryAlgorithm
{
public:
    SurfaceMeshSmoothen();
    ~SurfaceMeshSmoothen() override = default;

public:
    imstkGetMacro(NumberOfIterations, int);
    imstkGetMacro(RelaxationFactor, double);
    imstkGetMacro(Convergence, double);
    imstkGetMacro(FeatureAngle, double);
    imstkGetMacro(EdgeAngle, double);
    imstkGetMacro(FeatureEdgeSmoothing, bool);
    imstkGetMacro(BoundarySmoothing, bool);

    ///
    /// \brief Required input, port 0
    ///
    void setInputMesh(std::shared_ptr<SurfaceMesh> inputMesh);

    imstkSetMacro(NumberOfIterations, int);
    imstkSetMacro(RelaxationFactor, double);
    imstkSetMacro(Convergence, double);
    imstkSetMacro(FeatureAngle, double);
    imstkSetMacro(EdgeAngle, double);
    imstkSetMacro(FeatureEdgeSmoothing, bool);
    imstkSetMacro(BoundarySmoothing, bool);

protected:
    void requestUpdate() override;

private:
    int    m_NumberOfIterations = 20;
    double m_RelaxationFactor   = 0.01;
    double m_Convergence  = 0.0;
    double m_FeatureAngle = 45.0;
    double m_EdgeAngle    = 15.0;
    bool   m_FeatureEdgeSmoothing = false;
    bool   m_BoundarySmoothing    = true;
};
} // namespace imstk