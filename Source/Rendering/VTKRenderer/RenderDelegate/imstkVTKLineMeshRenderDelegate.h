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

#include "imstkVTKPolyDataRenderDelegate.h"

class vtkCellArray;
class vtkDataArray;
class vtkDoubleArray;
class vtkPolyData;

namespace imstk
{
class LineMesh;
template<typename T, int N> class VecDataArray;

///
/// \class VTKLineMeshRenderDelegate
///
/// \brief
///
class VTKLineMeshRenderDelegate : public VTKPolyDataRenderDelegate
{
public:
    ///
    /// \brief Constructor
    ///
    VTKLineMeshRenderDelegate(std::shared_ptr<VisualModel> visualModel);

    virtual ~VTKLineMeshRenderDelegate() override = default;

    ///
    /// \brief Event handler
    ///
    void processEvents() override;

protected:
    ///
    /// \brief Callback when vertices change
    ///
    void vertexDataModified(Event* e);

    ///
    /// \brief Callback when geometry changes
    ///
    void geometryModified(Event* e);

protected:
    std::shared_ptr<VecDataArray<double, 3>> m_vertices;
    std::shared_ptr<VecDataArray<int, 2>>    m_indices;

    vtkSmartPointer<vtkPolyData> m_polydata;

    vtkSmartPointer<vtkDoubleArray> m_mappedVertexArray;       ///> Mapped array of vertices
    vtkSmartPointer<vtkDataArray>   m_mappedVertexScalarArray; ///> Mapped array of scalars
    vtkSmartPointer<vtkCellArray>   m_cellArray;               ///> Array of cells
};
}
