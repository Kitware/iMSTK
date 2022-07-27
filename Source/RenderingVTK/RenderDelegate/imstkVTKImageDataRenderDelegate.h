/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkVTKVolumeRenderDelegate.h"

class vtkImageData;

namespace imstk
{
class AbstractDataArray;

///
/// \class VTKImageDataRenderDelegate
///
/// \brief Render delegate to do to volume rendering using VTK
///
class VTKImageDataRenderDelegate : public VTKVolumeRenderDelegate
{
public:
    VTKImageDataRenderDelegate(std::shared_ptr<VisualModel> visualModel);
    ~VTKImageDataRenderDelegate() override = default;

    ///
    /// \brief Update render delegate source based on the internal data
    ///
    void processEvents() override;

protected:
    ///
    /// \brief Callback for when the image is modified
    ///
    void imageDataModified(Event* e);

    ///
    /// \brief Callback for when the image scalars are modified
    ///
    void imageScalarsModified(Event* e);

    std::shared_ptr<AbstractDataArray> m_scalarArray;
    vtkSmartPointer<vtkImageData>      imageDataVtk;
};
} // namespace imstk