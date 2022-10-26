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
class vtkFloatArray;
class vtkUnstructuredGrid;

namespace imstk
{
class HexahedralMesh;
template<typename T, int N> class VecDataArray;

///
/// \class VTKHexahedralMeshRenderDelegate
///
/// \brief Delegates rendering of HexahedralMesh to VTK from VisualModel
///
class VTKHexahedralMeshRenderDelegate : public VTKPolyDataRenderDelegate
{
public:
    VTKHexahedralMeshRenderDelegate();
    ~VTKHexahedralMeshRenderDelegate() override = default;

    ///
    /// \brief Process Events
    ///
    void processEvents() override;

    void geometryModified(Event* e);

    void vertexDataModified(Event* e);

protected:
    void init() override;

    std::shared_ptr<VecDataArray<double, 3>> m_vertices;
    std::shared_ptr<VecDataArray<int, 8>>    m_indices;

    vtkSmartPointer<vtkUnstructuredGrid> m_mesh;

    vtkSmartPointer<vtkDoubleArray> m_mappedVertexArray;       ///< Mapped array of vertices
    vtkSmartPointer<vtkDataArray>   m_mappedVertexScalarArray; ///< Mapped array of scalars
    vtkSmartPointer<vtkCellArray>   m_cellArray;               ///< Array of cells
};
} // namespace imstk