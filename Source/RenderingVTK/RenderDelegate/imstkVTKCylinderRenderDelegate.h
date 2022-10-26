/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkVTKPolyDataRenderDelegate.h"

class vtkCylinderSource;

namespace imstk
{
///
/// \class VTKCylinderRenderDelegate
///
/// \brief Delegates rendering of Cylinders to VTK from VisualModel
///
class VTKCylinderRenderDelegate : public VTKPolyDataRenderDelegate
{
public:
    VTKCylinderRenderDelegate();
    ~VTKCylinderRenderDelegate() override = default;

    ///
    /// \brief Process the event queue
    ///
    void processEvents() override;

protected:
    void init() override;

    vtkSmartPointer<vtkCylinderSource> m_cylinderSource;
};
} // namespace imstk