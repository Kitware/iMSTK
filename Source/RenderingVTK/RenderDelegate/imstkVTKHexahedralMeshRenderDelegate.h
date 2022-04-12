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
class vtkFloatArray;
class vtkUnstructuredGrid;

namespace imstk
{
class HexahedralMesh;
template<typename T, int N> class VecDataArray;

///
/// \class VTKHexahedralMeshRenderDelegate
///
/// \brief Hexahedral mesh render delegate with VTK backend
///
class VTKHexahedralMeshRenderDelegate : public VTKPolyDataRenderDelegate
{
public:
    VTKHexahedralMeshRenderDelegate(std::shared_ptr<VisualModel> visualModel);
    ~VTKHexahedralMeshRenderDelegate() override = default;

    ///
    /// \brief Process Events
    ///
    void processEvents() override;

    void geometryModified(Event* e);

    void vertexDataModified(Event* e);

protected:
    std::shared_ptr<VecDataArray<double, 3>> m_vertices;
    std::shared_ptr<VecDataArray<int, 8>>    m_indices;

    vtkSmartPointer<vtkUnstructuredGrid> m_mesh;

    vtkSmartPointer<vtkDoubleArray> m_mappedVertexArray;       ///< Mapped array of vertices
    vtkSmartPointer<vtkDataArray>   m_mappedVertexScalarArray; ///< Mapped array of scalars
    vtkSmartPointer<vtkCellArray>   m_cellArray;               ///< Array of cells
};
} // namespace imstk