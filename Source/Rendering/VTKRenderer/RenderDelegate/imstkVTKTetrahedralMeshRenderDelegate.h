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

class vtkDoubleArray;
class vtkUnstructuredGrid;

namespace imstk
{
class TetrahedralMesh;

///
/// \class VTKTetrahedralMeshRenderDelegate
///
/// \brief Tetrahedral mesh render delegate with vtk render backend
///
class VTKTetrahedralMeshRenderDelegate : public VTKRenderDelegate
{
public:
    ///
    /// \brief Constructor
    ///
    explicit VTKTetrahedralMeshRenderDelegate(std::shared_ptr<VisualModel> visualModel);

    ///
    /// \brief Destructor
    ///
    virtual ~VTKTetrahedralMeshRenderDelegate() override = default;

    ///
    /// \brief Update unstructured grid source based on the tetrahedral mesh
    ///
    void updateDataSource() override;

protected:

    vtkSmartPointer<vtkDoubleArray>      m_mappedVertexArray; ///> Mapped array of vertices
    vtkSmartPointer<vtkUnstructuredGrid> m_mesh;              ///> Mapped tetrahedral mesh
};
} // imstk
