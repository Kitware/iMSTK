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
class vtkPolyData;

namespace imstk
{
class AbstractDataArray;
class SurfaceMesh;
template<typename T, int N> class VecDataArray;

///
/// \class VTKSurfaceMeshRenderDelegate
///
/// \brief Surface mesh render delegate with VTK backend.
///
class VTKSurfaceMeshRenderDelegate : public VTKPolyDataRenderDelegate
{
public:
    VTKSurfaceMeshRenderDelegate(std::shared_ptr<VisualModel> visualModel);
    ~VTKSurfaceMeshRenderDelegate() override = default;

    ///
    /// \brief Event handler
    ///
    void processEvents() override;

    ///
    /// \brief Initialize textures
    ///
    void initializeTextures();

// Callbacks for modifications, when an element changes the user or API must post the modified event
// to inform that this happened, if the actual buffer on the geometry is swapped then geometry
// modified would instead be called
protected:
    ///
    /// \brief Callback for when vertex values are modified
    ///
    void vertexDataModified(Event* e);
    void indexDataModified(Event* e);
    void normalDataModified(Event* e);
    void vertexScalarsModified(Event* e);
    void cellScalarsModified(Event* e);
    void textureCoordinatesModified(Event* e);

    ///
    /// \brief Callback for when geometry is modified
    ///
    void geometryModified(Event* e);

    ///
    /// \brief Callback for when RenderMaterial textures are modified
    ///
    void texturesModified(Event* e);

    void setVertexBuffer(std::shared_ptr<VecDataArray<double, 3>> vertices);
    void setNormalBuffer(std::shared_ptr<VecDataArray<double, 3>> normals);
    void setIndexBuffer(std::shared_ptr<VecDataArray<int, 3>> indices);
    void setVertexScalarBuffer(std::shared_ptr<AbstractDataArray> scalars);
    void setCellScalarBuffer(std::shared_ptr<AbstractDataArray> scalars);
    void setTextureCoordinateBuffer(std::shared_ptr<AbstractDataArray> textureCoordinates);

    std::shared_ptr<SurfaceMesh> m_geometry;
    bool m_isDynamicMesh;

    std::shared_ptr<VecDataArray<double, 3>> m_vertices;
    std::shared_ptr<VecDataArray<double, 3>> m_normals;
    std::shared_ptr<VecDataArray<int, 3>>    m_indices;
    std::shared_ptr<AbstractDataArray>       m_vertexScalars;
    std::shared_ptr<AbstractDataArray>       m_cellScalars;
    std::shared_ptr<AbstractDataArray>       m_textureCoordinates;

    vtkSmartPointer<vtkPolyData> m_polydata;

    vtkSmartPointer<vtkDoubleArray> m_mappedVertexArray;       ///< Mapped array of vertices
    vtkSmartPointer<vtkDoubleArray> m_mappedNormalArray;       ///< Mapped array of normals
    vtkSmartPointer<vtkFloatArray>  m_mappedTangentArray;      ///< Mapped array of tangents
    vtkSmartPointer<vtkFloatArray>  m_mappedTCoordsArray;      ///< Mapped array of tcoords
    vtkSmartPointer<vtkDataArray>   m_mappedVertexScalarArray; ///< Mapped array of scalars
    vtkSmartPointer<vtkDataArray>   m_mappedCellScalarArray;   ///< Mapped array of scalars
    vtkSmartPointer<vtkCellArray>   m_cellArray;               ///< Array of cells
};
} // namespace imstk
