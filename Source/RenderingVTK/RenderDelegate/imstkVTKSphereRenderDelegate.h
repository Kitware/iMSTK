/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkVTKPolyDataRenderDelegate.h"

class vtkSphereSource;

namespace imstk
{
///
/// \class VTKSphereRenderDelegate
///
/// \brief Delegates rendering of Sphere to VTK from VisualModel
///
class VTKSphereRenderDelegate : public VTKPolyDataRenderDelegate
{
public:
    VTKSphereRenderDelegate();
    ~VTKSphereRenderDelegate() override = default;

    ///
    /// \brief Update sphere source based on the sphere geometry
    ///
    void processEvents() override;

protected:
    void init() override;

    vtkSmartPointer<vtkSphereSource> m_sphereSource;
};
} // namespace imstk