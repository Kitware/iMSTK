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
/// \brief Cylinder object render delegate with VTK backend
///
class VTKCylinderRenderDelegate : public VTKPolyDataRenderDelegate
{
public:
    VTKCylinderRenderDelegate(std::shared_ptr<VisualModel> visualModel);
    ~VTKCylinderRenderDelegate() override = default;

    ///
    /// \brief Process the event queue
    ///
    void processEvents() override;

protected:
    vtkSmartPointer<vtkCylinderSource> m_cylinderSource;
};
} // namespace imstk