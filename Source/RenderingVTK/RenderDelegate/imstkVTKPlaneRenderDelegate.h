/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkVTKPolyDataRenderDelegate.h"

class vtkPlaneSource;

namespace imstk
{
///
/// \class VTKPlaneRenderDelegate
///
/// \brief Delegates rendering of Plane to VTK from VisualModel
///
class VTKPlaneRenderDelegate : public VTKPolyDataRenderDelegate
{
public:
    VTKPlaneRenderDelegate();
    ~VTKPlaneRenderDelegate() override = default;

    ///
    /// \brief Update plane source based on the plane geometry
    ///
    void processEvents() override;

protected:
    void init() override;

    vtkSmartPointer<vtkPlaneSource> m_planeSource;
};
} // namespace imstk