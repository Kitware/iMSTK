/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkVTKRenderDelegate.h"

namespace imstk
{
///
/// \class VTKPolyDataRenderDelegate
///
/// \brief Abstract base class for PolyData-based RenderDelegates
///
class VTKPolyDataRenderDelegate : public VTKRenderDelegate
{
public:
    ~VTKPolyDataRenderDelegate() override = default;

protected:
    VTKPolyDataRenderDelegate(std::shared_ptr<VisualModel> visualModel) : VTKRenderDelegate(visualModel) { }

    ///
    /// \brief Updates the actor and mapper properties from the currently set VisualModel
    ///
    void updateRenderProperties() override;
};
} // namespace imstk