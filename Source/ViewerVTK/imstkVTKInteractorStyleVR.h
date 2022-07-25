/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include <vtkInteractorStyle3D.h>
#include <memory>

namespace imstk
{
class OpenVRDeviceClient;
} // namespace imstk

///
/// \class vtkInteractorStyleVR
///
/// \brief VTK Interactor style for VR
///
class vtkInteractorStyleVR : public vtkInteractorStyle3D
{
public:
    static vtkInteractorStyleVR* New();
    vtkTypeMacro(vtkInteractorStyleVR, vtkInteractorStyle3D);

    void OnMove3D(vtkEventData* edata) override;

    ///
    /// \brief Adds button actions
    ///
    void addButtonActions();

    ///
    /// \brief Adds thumbstick movement actions
    ///
    void addMovementActions();

    std::shared_ptr<imstk::OpenVRDeviceClient> getLeftControllerDeviceClient() const { return m_leftControllerDeviceClient; }
    std::shared_ptr<imstk::OpenVRDeviceClient> getRightControllerDeviceClient() const { return m_rightControllerDeviceClient; }
    std::shared_ptr<imstk::OpenVRDeviceClient> getHmdDeviceClient() const { return m_hmdDeviceClient; }

    vtkInteractorStyleVR();

protected:
    void OnButtonPress(vtkEventData* data, int buttonId);

public:
    std::shared_ptr<imstk::OpenVRDeviceClient> m_leftControllerDeviceClient;
    std::shared_ptr<imstk::OpenVRDeviceClient> m_rightControllerDeviceClient;
    std::shared_ptr<imstk::OpenVRDeviceClient> m_hmdDeviceClient;
};
