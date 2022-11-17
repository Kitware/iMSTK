/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkVTKVolumeRenderDelegate.h"

class vtkDoubleArray;
class vtkPolyData;

namespace imstk
{
template<typename T, int N> class VecDataArray;

///
/// \class VTKFluidRenderDelegate
///
/// \brief Delegates rendering of Fluid to VTK from VisualModel
///
class VTKFluidRenderDelegate : public VTKVolumeRenderDelegate
{
public:
    VTKFluidRenderDelegate();
    ~VTKFluidRenderDelegate() override = default;

    ///
    /// \brief Update polydata source based on the mesh geometry
    ///
    void processEvents() override;

protected:
    void init() override;

    ///
    /// \brief Callback for when geometry changes
    ///
    void geometryModified(Event* e);

    ///
    /// \brief Callback for when vertices change
    ///
    void vertexDataModified(Event* e);

    void updateRenderProperties() override;

    std::shared_ptr<VecDataArray<double, 3>> m_vertices;

    vtkSmartPointer<vtkPolyData> m_polydata;

    vtkSmartPointer<vtkDoubleArray> m_mappedVertexArray; ///< Mapped array of vertices
};
} // namespace imstk
