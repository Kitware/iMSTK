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
/// \brief Sphere object render delegate with VTK backend
///
class VTKSphereRenderDelegate : public VTKPolyDataRenderDelegate
{
public:
    VTKSphereRenderDelegate(std::shared_ptr<VisualModel> visualModel);
    ~VTKSphereRenderDelegate() override = default;

    ///
    /// \brief Update sphere source based on the sphere geometry
    ///
    void processEvents() override;

protected:
    vtkSmartPointer<vtkSphereSource> m_sphereSource;
};
} // namespace imstk