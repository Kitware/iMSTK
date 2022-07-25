/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkScreenCaptureUtility.h"

#include <vtkSmartPointer.h>

class vtkPNGWriter;
class vtkRenderWindow;
class vtkWindowToImageFilter;

namespace imstk
{
///
/// \class VTKScreenCaptureUtility
///
/// \brief Utility class to manage screen capture through VTK
///
class VTKScreenCaptureUtility : public ScreenCaptureUtility
{
public:
    using ScreenCaptureUtility::saveScreenShot;

    VTKScreenCaptureUtility(vtkRenderWindow* const rw, const std::string prefix = "Screenshot-");
    ~VTKScreenCaptureUtility() override = default;

    ///
    /// \brief Saves the screenshot as a png file
    ///
    virtual std::string saveScreenShot(const std::string& captureName) override;

protected:
    vtkSmartPointer<vtkWindowToImageFilter> m_windowToImageFilter;
    vtkSmartPointer<vtkPNGWriter> m_pngWriter; ///< using vtk's png writer to save the screenshots
    vtkRenderWindow* m_renderWindow;           ///< render window whose screen shot will be taken
};
} // namespace imstk