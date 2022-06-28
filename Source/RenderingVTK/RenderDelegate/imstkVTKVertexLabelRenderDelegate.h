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

class vtkDoubleArray;
class vtkPolyData;

namespace imstk
{
class PointSet;
template<typename T, int N> class VecDataArray;

///
/// \class VTKVertexLabelRenderDelegate
///
/// \brief Render delegate for vertex labels.
///
class VTKVertexLabelRenderDelegate : public VTKRenderDelegate
{
public:
    VTKVertexLabelRenderDelegate(std::shared_ptr<VisualModel> visualModel);
    ~VTKVertexLabelRenderDelegate() override = default;

    ///
    /// \brief Update polydata source based on the mesh geometry
    ///
    void processEvents() override;

    void updateRenderProperties() override;

protected:
    ///
    /// \brief Callback for when vertex data changes
    ///
    void vertexDataModified(Event* e);

    ///
    /// \brief Callback for when geometry changes
    ///
    void geometryModified(Event* e);

    void setVertexBuffer(std::shared_ptr<VecDataArray<double, 3>> vertices);

    std::shared_ptr<PointSet> m_geometry;
    std::shared_ptr<VecDataArray<double, 3>> m_vertices;

    vtkSmartPointer<vtkPolyData> m_polydata;

    vtkSmartPointer<vtkDoubleArray> m_mappedVertexArray; ///< Mapped array of vertices
};
} // namespace imstk
