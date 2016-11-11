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

#ifndef imstkVTKSurfaceMeshRenderDelegate_h
#define imstkVTKSurfaceMeshRenderDelegate_h

#include <memory>

#include "imstkVTKRenderDelegate.h"
#include "imstkSurfaceMesh.h"

#include "vtkPolyData.h"
#include "imstkVTKMappedVertexArray.h"

namespace imstk
{

///
/// \class SurfaceMeshRenderDelegate
///
/// \brief
///
class VTKSurfaceMeshRenderDelegate : public VTKRenderDelegate
{
public:
    ///
    /// \brief
    ///
    ~VTKSurfaceMeshRenderDelegate() = default;

    ///
    /// \brief
    ///
    VTKSurfaceMeshRenderDelegate(std::shared_ptr<SurfaceMesh>SurfaceMesh);

    ///
    /// \brief
    ///
    void mapVertices();


    ///
    /// \brief
    ///
    void update();

    ///
    /// \brief
    ///
    std::shared_ptr<Geometry>getGeometry() const override;

protected:

    std::shared_ptr<SurfaceMesh> m_geometry;
    vtkSmartPointer<vtkDoubleArray> m_mappedVertexArray;

};

}

#endif // ifndef imstkSurfaceMeshRenderDelegate_h
