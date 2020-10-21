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
#include "imstkDataArray.h"

namespace imstk
{
class PointSet;
class SurfaceMesh;

///
/// \class CleanMesh
///
/// \brief This filter will select the points within a surfacemesh. It can
/// either prune or return the mask
///
class SelectEnclosedPoints : public GeometryAlgorithm
{
public:
    SelectEnclosedPoints();
    virtual ~SelectEnclosedPoints() override = default;

public:
    void setInputMesh(std::shared_ptr<SurfaceMesh> inputMesh);
    void setInputPoints(std::shared_ptr<PointSet> inputPoints);
    std::shared_ptr<PointSet> getOutputPoints() const;

    imstkGetMacro(UsePruning, bool);
    imstkGetMacro(Tolerance, double);
    imstkGetMacro(InsideOut, bool);
    imstkGetMacro(IsInsideMask, std::shared_ptr<DataArray<unsigned char>>);

    imstkSetMacro(UsePruning, bool);
    imstkSetMacro(Tolerance, double);
    imstkSetMacro(InsideOut, bool);

protected:
    void requestUpdate() override;

private:
    bool   m_UsePruning = true;
    double m_Tolerance  = 0.0;
    bool   m_InsideOut  = false;
    std::shared_ptr<DataArray<unsigned char>> m_IsInsideMask = nullptr;
};
}