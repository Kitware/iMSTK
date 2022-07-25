/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkVTKPolyDataRenderDelegate.h"

class vtkCapsuleSource;

namespace imstk
{
///
/// \class VTKCapsuleRenderDelegate
///
/// \brief Render capsule object with vtk backend
///
class VTKCapsuleRenderDelegate : public VTKPolyDataRenderDelegate
{
public:
    VTKCapsuleRenderDelegate(std::shared_ptr<VisualModel> visualModel);
    ~VTKCapsuleRenderDelegate() override = default;

    ///
    /// \brief Update capsule source based on the capsule geometry
    ///
    void processEvents() override;

protected:
    vtkSmartPointer<vtkCapsuleSource> m_capsuleSource;
};
} // namespace imstk