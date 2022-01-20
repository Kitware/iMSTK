/*=========================================================================

   Library: iMSTK

   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
   & Imaging in Medicine, Rensselaer Polytechnic Institute.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

          http://www.apache.org/licenses/LICENSE-2.0.txt

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

=========================================================================*/
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