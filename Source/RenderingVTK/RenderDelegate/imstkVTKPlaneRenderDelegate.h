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
/// \brief Plane object render delegate
///
class VTKPlaneRenderDelegate : public VTKPolyDataRenderDelegate
{
public:
    VTKPlaneRenderDelegate(std::shared_ptr<VisualModel> visualModel);
    ~VTKPlaneRenderDelegate() override = default;

    ///
    /// \brief Update plane source based on the plane geometry
    ///
    void processEvents() override;

protected:
    vtkSmartPointer<vtkPlaneSource> m_planeSource;
};
} // namespace imstk