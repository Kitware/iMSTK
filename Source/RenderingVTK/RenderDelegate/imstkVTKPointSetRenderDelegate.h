/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkVTKPolyDataRenderDelegate.h"

class vtkDataArray;
class vtkDoubleArray;
class vtkPolyData;

namespace imstk
{
class AbstractDataArray;
class PointSet;
template<typename T, int N> class VecDataArray;

///
/// \class VTKPointSetRenderDelegate
///
/// \brief Render delegate for point set. A 3D glyph of spheres is
/// created to render each node
///
class VTKPointSetRenderDelegate : public VTKPolyDataRenderDelegate
{
public:
    VTKPointSetRenderDelegate(std::shared_ptr<VisualModel> visualModel);
    ~VTKPointSetRenderDelegate() override = default;

    ///
    /// \brief Update polydata source based on the mesh geometry
    ///
    void processEvents() override;

// Callbacks for modifications, when an element changes the user or API must post the modified event
// to inform that this happened, if the actual buffer on the geometry is swapped then geometry
// modified would instead be called
protected:
    ///
    /// \brief Callback for when vertex data changes
    ///
    void vertexDataModified(Event* e);
    void vertexScalarsModified(Event* e);

    ///
    /// \brief Callback for when geometry changes
    ///
    void geometryModified(Event* e);

    void setVertexBuffer(std::shared_ptr<VecDataArray<double, 3>> vertices);
    void setVertexScalarBuffer(std::shared_ptr<AbstractDataArray> scalars);

    std::shared_ptr<PointSet> m_geometry;
    std::shared_ptr<VecDataArray<double, 3>> m_vertices;
    std::shared_ptr<AbstractDataArray>       m_vertexScalars;

    vtkSmartPointer<vtkPolyData> m_polydata;

    vtkSmartPointer<vtkDoubleArray> m_mappedVertexArray;       ///< Mapped array of vertices
    vtkSmartPointer<vtkDataArray>   m_mappedVertexScalarArray; ///< Mapped array of scalars
};
} // namespace imstk
