/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkVTKPolyDataRenderDelegate.h"

class vtkDataArray;
class vtkDoubleArray;
class vtkUnstructuredGrid;
class vtkCellArray;

namespace imstk
{
class TetrahedralMesh;
template<typename T, int N> class VecDataArray;

///
/// \class VTKTetrahedralMeshRenderDelegate
///
/// \brief Delegates rendering of TetrahedralMesh to VTK from VisualModel
///
class VTKTetrahedralMeshRenderDelegate : public VTKPolyDataRenderDelegate
{
public:
    VTKTetrahedralMeshRenderDelegate();
    ~VTKTetrahedralMeshRenderDelegate() override = default;

    ///
    /// \brief Process handling of messages recieved
    ///
    void processEvents() override;

protected:
    void init() override;

    ///
    /// \brief Callback for when vertex values are modified
    ///
    void vertexDataModified(Event* e);
    void indexDataModified(Event* e);

    ///
    /// \brief Callback for when geometry is modified
    ///
    void geometryModified(Event* e);

    void setVertexBuffer(std::shared_ptr<VecDataArray<double, 3>> vertices);
    void setIndexBuffer(std::shared_ptr<VecDataArray<int, 4>> indices);

    std::shared_ptr<VecDataArray<double, 3>> m_vertices;
    std::shared_ptr<VecDataArray<int, 4>>    m_indices;

    vtkSmartPointer<vtkUnstructuredGrid> m_mesh;               ///< Mapped tetrahedral mesh

    vtkSmartPointer<vtkDoubleArray> m_mappedVertexArray;       ///< Mapped array of vertices
    vtkSmartPointer<vtkDataArray>   m_mappedVertexScalarArray; ///< Mapped array of scalars
    vtkSmartPointer<vtkCellArray>   m_cellArray;               ///< Array of cells
};
} // namespace imstk