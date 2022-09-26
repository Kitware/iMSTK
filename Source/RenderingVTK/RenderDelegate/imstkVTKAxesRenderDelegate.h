/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkVTKRenderDelegate.h"

class vtkSphereSource;
class vtkAxesActor;

namespace imstk
{
///
/// \class VTKAxesRenderDelegate
///
/// \brief Axes render delegate with VTK backend
///
class VTKAxesRenderDelegate : public VTKRenderDelegate
{
public:
    VTKAxesRenderDelegate(std::shared_ptr<VisualModel> visualModel);
    ~VTKAxesRenderDelegate() override = default;

    ///
    /// \brief Update sphere source based on the sphere geometry
    ///
    void processEvents() override;
    void updateRenderProperties() override { }

protected:
    vtkSmartPointer<vtkAxesActor> m_axesActor;
};
} // namespace imstk