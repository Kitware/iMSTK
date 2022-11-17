/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

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
/// \brief Delegates rendering of text per PointSet vertex to VTK from VisualModel
///
class VTKVertexLabelRenderDelegate : public VTKRenderDelegate
{
public:
    VTKVertexLabelRenderDelegate();
    ~VTKVertexLabelRenderDelegate() override = default;

    ///
    /// \brief Update polydata source based on the mesh geometry
    ///
    void processEvents() override;

    void updateRenderProperties() override;

protected:
    void init() override;

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
