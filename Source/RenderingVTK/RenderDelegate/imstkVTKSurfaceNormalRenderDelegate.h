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

class vtkDataArray;
class vtkPolyData;

namespace imstk
{
class PointSet;
template<typename T, int N> class VecDataArray;

///
/// \class VTKSurfaceNormalRenderDelegate
///
/// \brief Renders normals as vectors from a surface mesh
///
class VTKSurfaceNormalRenderDelegate : public VTKPolyDataRenderDelegate
{
public:
    VTKSurfaceNormalRenderDelegate(std::shared_ptr<VisualModel> visualModel);
    ~VTKSurfaceNormalRenderDelegate() override = default;

    ///
    /// \brief Update polydata source based on the mesh geometry
    ///
    void processEvents() override;

protected:
    ///
    /// \brief Callback for when vertex data changes
    ///
    void vertexDataModified(Event* e);

    ///
    /// \brief Callback for when geometry changes
    ///
    void geometryModified(Event* e);

    ///
    /// \brief Updates the actor and mapper properties from the currently set VisualModel
    ///
    void updateRenderProperties() override;

    ///
    /// \brief Computes triangle centers from the current geometry
    ///
    std::shared_ptr<VecDataArray<double, 3>> computeTriangleCenters(
        std::shared_ptr<VecDataArray<double, 3>> verticesPtr,
        std::shared_ptr<VecDataArray<int, 3>> indicesPtr);

    std::shared_ptr<VecDataArray<double, 3>> computeTriangleNormals(
        std::shared_ptr<VecDataArray<double, 3>> verticesPtr,
        std::shared_ptr<VecDataArray<int, 3>> indicesPtr);

    std::shared_ptr<VecDataArray<double, 3>> m_surfMeshVertices;
    std::shared_ptr<VecDataArray<int, 3>>    m_surfMeshIndices;

    std::shared_ptr<VecDataArray<double, 3>> m_triangleCenterVertices;
    std::shared_ptr<VecDataArray<double, 3>> m_triangleNormals;

    vtkSmartPointer<vtkPolyData> m_polydata;
    vtkSmartPointer<vtkPolyData> m_glyphPolyData;

    vtkSmartPointer<vtkDataArray> m_mappedVertexArray;  ///> Mapped array of vertices
    vtkSmartPointer<vtkDataArray> m_mappedNormalsArray; ///> Mapped array of orientations
};
} // namespace imstk
