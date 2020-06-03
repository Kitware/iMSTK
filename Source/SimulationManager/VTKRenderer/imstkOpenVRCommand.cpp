/*=========================================================================

Library: iMSTK

Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
& Imaging in Medicine, Rensselaer Polytechnic Institute.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0.txt

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

=========================================================================*/

// vtk
#include <vtkObjectFactory.h>

// imstk
#include "imstkOpenVRCommand.h"
#include "imstkSimulationManager.h"
#include "VTKRenderer/imstkVTKRenderer.h"
#include "imstkCamera.h"

namespace imstk
{
void
OpenVRCommand::Execute(
    vtkObject*    caller,
    unsigned long eventId,
    void*         callData)
{
    /// \todo Clear warnings  caller, callData are not used!
    // prevent warnings
    caller;
    callData;

    if (!m_simManager)
    {
        return;
    }

    // trigger button
    if (eventId == vtkCommand::CharEvent)
    {
        SimulationStatus status = m_simManager->getStatus();

        if (status == SimulationStatus::Running)
        {
            m_simManager->pause();
        }

        // pause simulation
        if (status == SimulationStatus::Inactive)
        {
            m_simManager->start();
        }

        // continue simulation
        if (status == SimulationStatus::Paused)
        {
            m_simManager->run();
        }

        this->AbortFlagOn();
    }

    if (eventId == vtkCommand::LeftButtonPressEvent)
    {
        SimulationStatus status = m_simManager->getStatus();

        // pause simulation
        if (status == SimulationStatus::Running)
        {
            m_simManager->pause();
        }

        // continue simulation
        if (status == SimulationStatus::Paused)
        {
            m_simManager->run();
        }

        this->AbortFlagOn();
    }

    // home button
    if (eventId == vtkCommand::FourthButtonReleaseEvent)
    {
        SimulationStatus status = m_simManager->getStatus();

        if (status == SimulationStatus::Inactive)
        {
            m_simManager->start();
        }
        else
        {
            m_simManager->end();
        }

        this->AbortFlagOn();
    }

    // Pad
    if (eventId == vtkCommand::RightButtonPressEvent)
    {
        SimulationStatus status = m_simManager->getStatus();
        // desactivate pad if in simulation mode
        if (status != SimulationStatus::Inactive)
        {
            this->AbortFlagOn();
        }
    }

    // grip button
    if (eventId == vtkCommand::MiddleButtonPressEvent)
    {
        /// \todo Take a screenshot

        this->AbortFlagOn();
    }

    // Update render delagates after each rendering
    // same as subclassing vtkOpenVRRenderWindowInteractor and
    // override DoOneEvent() by calling itself and then updateRenderDelegates()
    if (eventId == vtkCommand::StartEvent)
    {
        //SimulationStatus status = m_simManager->getStatus();

#ifndef iMSTK_USE_Vulkan
        std::dynamic_pointer_cast<VTKRenderer>(m_simManager->getViewer()->getActiveRenderer())->updateRenderDelegates();
#endif
    }
    else
    {
        //int a = eventId;
    }
}
} // imstk
