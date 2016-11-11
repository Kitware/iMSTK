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

#ifndef imstkVTKTetrahedralMeshRenderDelegate_h
#define imstkVTKTetrahedralMeshRenderDelegate_h

#include <memory>

#include "imstkVTKRenderDelegate.h"
#include "imstkTetrahedralMesh.h"

#include "vtkUnstructuredGrid.h"

namespace imstk
{

///
/// \class TetrahedralMeshRenderDelegate
///
/// \brief
///
class VTKTetrahedralMeshRenderDelegate : public VTKRenderDelegate
{
public:
    ///
    /// \brief
    ///
    ~VTKTetrahedralMeshRenderDelegate() = default;

    ///
    /// \brief
    ///
    VTKTetrahedralMeshRenderDelegate(std::shared_ptr<TetrahedralMesh> tetrahedralMesh);

    ///
    /// \brief
    ///
    std::shared_ptr<Geometry> getGeometry() const override;

protected:
    std::shared_ptr<TetrahedralMesh> m_geometry;    ///>
};

} // imstk

#endif // ifndef imstkTetrahedralMeshRenderDelegate_h
