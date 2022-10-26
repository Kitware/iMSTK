/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkVTKRenderDelegate.h"

class vtkAxesActor;

namespace imstk
{
///
/// \class VTKAxesRenderDelegate
///
/// \brief Delegates rendering of Axes to VTK from VisualModel
///
class VTKAxesRenderDelegate : public VTKRenderDelegate
{
public:
    VTKAxesRenderDelegate();
    ~VTKAxesRenderDelegate() override = default;

    ///
    /// \brief Update sphere source based on the sphere geometry
    ///
    void processEvents() override;
    void updateRenderProperties() override { }

protected:
    void init() override;

    vtkSmartPointer<vtkAxesActor> m_axesActor;
};
} // namespace imstk