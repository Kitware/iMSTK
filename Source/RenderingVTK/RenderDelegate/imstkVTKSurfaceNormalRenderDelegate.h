/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

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

    vtkSmartPointer<vtkDataArray> m_mappedVertexArray;  ///< Mapped array of vertices
    vtkSmartPointer<vtkDataArray> m_mappedNormalsArray; ///< Mapped array of orientations
};
} // namespace imstk
