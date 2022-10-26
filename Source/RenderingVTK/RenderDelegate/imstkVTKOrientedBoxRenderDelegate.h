/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkVTKPolyDataRenderDelegate.h"

class vtkCubeSource;

namespace imstk
{
///
/// \class VTKOrientedCubeRenderDelegate
///
/// \brief Delegates rendering of OrientedBox to VTK from VisualModel
///
class VTKOrientedCubeRenderDelegate : public VTKPolyDataRenderDelegate
{
public:
    VTKOrientedCubeRenderDelegate();
    ~VTKOrientedCubeRenderDelegate() override = default;

    ///
    /// \brief Update cube source based on the cube geometry
    ///
    void processEvents() override;

protected:
    void init() override;

    vtkSmartPointer<vtkCubeSource> m_cubeSource;
};
} // namespace imstk