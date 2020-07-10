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

#include "imstkVTKRenderDelegate.h"

class vtkCellArray;
class vtkDoubleArray;
class vtkPoints;
class vtkPolyData;

namespace imstk
{
class SurfaceMesh;

///
/// \class VTKdbgTrianglesRenderDelegate
///
/// \brief Render delegate for a set of disjoint triangles whose size could change
///  more frequently which is typically he case for rendering arbitrary set of triangles
///
class VTKdbgTrianglesRenderDelegate : public VTKRenderDelegate
{
public:
    ///
    /// \brief Constructor
    ///
    explicit VTKdbgTrianglesRenderDelegate(std::shared_ptr<VisualModel> visualModel);

    virtual ~VTKdbgTrianglesRenderDelegate() override = default;

    ///
    /// \brief Update polydata source based on the surface mesh geometry
    ///
    void updateDataSource() override;

protected:

    // Auxiliary variables for rendering
    vtkSmartPointer<vtkDoubleArray> m_paddedVertexArray;
    vtkSmartPointer<vtkPoints>      m_points;
    vtkSmartPointer<vtkCellArray>   m_cellArray;
    vtkSmartPointer<vtkPolyData>    m_polyData;
};
}
