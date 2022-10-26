/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

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
class AbstractDataArray;

///
/// \class VTKLineMeshRenderDelegate
///
/// \brief Delegates rendering of LineMesh to VTK from VisualModel
///
class VTKLineMeshRenderDelegate : public VTKPolyDataRenderDelegate
{
public:
    VTKLineMeshRenderDelegate();
    ~VTKLineMeshRenderDelegate() override = default;

    ///
    /// \brief Event handler
    ///
    void processEvents() override;

// Callbacks for modifications, when an element changes the user or API must post the modified event
// to inform that this happened, if the actual buffer on the geometry is swapped then geometry
// modified would instead be called
protected:
    void init() override;

    ///
    /// \brief Callback for when vertex values are modified
    ///
    void vertexDataModified(Event* e);
    void indexDataModified(Event* e);
    void vertexScalarsModified(Event* e);
    void cellScalarsModified(Event* e);

    ///
    /// \brief Callback when geometry changes
    ///
    void geometryModified(Event* e);

    void setVertexBuffer(std::shared_ptr<VecDataArray<double, 3>> vertices);
    void setIndexBuffer(std::shared_ptr<VecDataArray<int, 2>> indices);
    void setVertexScalarBuffer(std::shared_ptr<AbstractDataArray> scalars);
    void setCellScalarBuffer(std::shared_ptr<AbstractDataArray> scalars);

    std::shared_ptr<LineMesh> m_geometry;
    std::shared_ptr<VecDataArray<double, 3>> m_vertices;
    std::shared_ptr<VecDataArray<int, 2>>    m_indices;
    std::shared_ptr<AbstractDataArray>       m_vertexScalars;
    std::shared_ptr<AbstractDataArray>       m_cellScalars;

    vtkSmartPointer<vtkPolyData> m_polydata;

    vtkSmartPointer<vtkDoubleArray> m_mappedVertexArray;       ///< Mapped array of vertices
    vtkSmartPointer<vtkDataArray>   m_mappedVertexScalarArray; ///< Mapped array of scalars
    vtkSmartPointer<vtkDataArray>   m_mappedCellScalarArray;   ///< Mapped array of scalars
    vtkSmartPointer<vtkCellArray>   m_cellArray;               ///< Array of cells
};
} // namespace imstk
