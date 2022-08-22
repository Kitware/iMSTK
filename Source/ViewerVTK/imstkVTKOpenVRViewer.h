/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkAbstractVTKViewer.h"
#include "imstkMacros.h"

#include <vtkSmartPointer.h>
#include <list>

class vtkRenderWindow;

namespace imstk
{
class OpenVRDeviceClient;

///
/// \class VTKViewer
///
/// \brief Subclasses viewer for the VTK rendering back-end
/// Creates vtk renderer for each scene. Forwards mouse and keyboard events
/// to the vtk renderwindow
///
class VTKOpenVRViewer : public AbstractVTKViewer
{
public:
    VTKOpenVRViewer(std::string name = "VTKOpenVRViewer");
    ~VTKOpenVRViewer() override      = default;

    IMSTK_TYPE_NAME(VTKOpenVRViewer)

    ///
    /// \brief Destructor
    ///
    void setRenderingMode(const Renderer::Mode mode) override;

    ///
    /// \brief Set scene to be rendered
    ///
    void setActiveScene(std::shared_ptr<Scene> scene) override;

    ///
    /// \brief Transform to physical space
    ///
    void setPhysicalToWorldTransform(const Mat4d& physicalToWorldMatrix);

    ///
    /// \brief Get transform to physical space
    ///
    Mat4d getPhysicalToWorldTransform();

    ///
    /// \brief Get one of the device clients for VR
    ///
    std::shared_ptr<OpenVRDeviceClient> getVRDeviceClient(int deviceType);

    ///
    /// \brief Acquire the full list of VR devices tied to this viewer
    ///
    const std::list<std::shared_ptr<OpenVRDeviceClient>>& getVRDeviceClients() const { return m_vrDeviceClients; }

    ///
    /// \brief VTKOpenVRViewer overrides to provide a non-rendering
    /// event processing loop (to deal with vsync blockage)
    ///
    void processEvents() override;

protected:
    bool initModule() override;

    void updateModule() override;

    ///< The VR controllers are tied to the view
    std::list<std::shared_ptr<OpenVRDeviceClient>> m_vrDeviceClients;
};
} // imstk
