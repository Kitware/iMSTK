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
/// \class CleanMesh
///
/// \brief This filter can merge duplicate points and cells, it only works with
/// LineMesh and SurfaceMesh. It accepts a tolerance as a fraction of the length
/// of bounding box of the input data or an absolute tolerance
///
class CleanMesh : public GeometryAlgorithm
{
public:
    CleanMesh();
    virtual ~CleanMesh() override = default;

    std::shared_ptr<SurfaceMesh> getOutputMesh() const;

    ///
    /// \brief Required input, port 0
    ///
    void setInputMesh(std::shared_ptr<SurfaceMesh> inputMesh);

    imstkGetMacro(Tolerance, double);
    imstkGetMacro(AbsoluteTolerance, double);
    imstkGetMacro(UseAbsolute, bool);

    ///
    /// \brief Set the tolerance for point merging, fraction of bounding box length
    ///
    void setTolerance(const double tolerance)
    {
        this->m_Tolerance = tolerance;
        m_UseAbsolute     = false;
    }

    void setAbsoluteTolerance(const double tolerance)
    {
        this->m_AbsoluteTolerance = tolerance;
        m_UseAbsolute = true;
    }

protected:
    void requestUpdate() override;

private:
    double m_Tolerance = 0.0;
    double m_AbsoluteTolerance = 1.0;
    bool   m_UseAbsolute       = false;
};
}