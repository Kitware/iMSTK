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

#ifndef imstkVTKInteractorStyle_h
#define imstkVTKInteractorStyle_h

#include <memory>
#include <chrono>

#include "vtkInteractorStyleTrackballCamera.h"

namespace imstk
{

class SimulationManager;

using vtkBaseInteractorStyle = vtkInteractorStyleTrackballCamera;

///
/// \class classname
///
/// \brief
///
class VTKInteractorStyle : public vtkBaseInteractorStyle
{
public:

    static VTKInteractorStyle *New();
    vtkTypeMacro(VTKInteractorStyle, vtkBaseInteractorStyle);

    ///
    /// \brief
    ///
    virtual void OnTimer() override;

    ///
    /// \brief
    ///
    virtual void OnChar() override;

    ///
    /// \brief
    ///
    virtual void OnMouseMove() override;

    ///
    /// \brief
    ///
    virtual void OnLeftButtonDown() override;

    ///
    /// \brief
    ///
    virtual void OnLeftButtonUp() override;

    ///
    /// \brief
    ///
    virtual void OnMiddleButtonDown() override;

    ///
    /// \brief
    ///
    virtual void OnMiddleButtonUp() override;

    ///
    /// \brief
    ///
    virtual void OnRightButtonDown() override;

    ///
    /// \brief
    ///
    virtual void OnRightButtonUp() override;

    ///
    /// \brief
    ///
    virtual void OnMouseWheelForward() override;

    ///
    /// \brief
    ///
    virtual void OnMouseWheelBackward() override;

    ///
    /// \brief
    ///
    void setSimulationManager(SimulationManager* simManager);

protected:

    friend class VTKViewer;

    ///
    /// \brief
    ///
    double getTargetFrameRate() const;

    ///
    /// \brief
    ///
    void setTargetFrameRate(const double& fps);

private:

    SimulationManager* m_simManager;
    double m_targetMS = 0.0;
    std::chrono::high_resolution_clock::time_point m_pre;
    std::chrono::high_resolution_clock::time_point m_post;

};

} // imstk

#endif // ifndef imstkInteractorStyle_h
