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

#ifndef imstkInteractorStyle_h
#define imstkInteractorStyle_h

#include <memory>

#include "vtkInteractorStyleTrackballCamera.h"

namespace imstk {
class SimulationManager;

using vtkBaseInteractorStyle = vtkInteractorStyleTrackballCamera;
class InteractorStyle : public vtkBaseInteractorStyle
{
public:

    static InteractorStyle *New();
    vtkTypeMacro(InteractorStyle, vtkBaseInteractorStyle);

    virtual void OnTimer() override;
    virtual void OnChar() override;
    virtual void OnMouseMove() override;
    virtual void OnLeftButtonDown() override;
    virtual void OnLeftButtonUp() override;
    virtual void OnMiddleButtonDown() override;
    virtual void OnMiddleButtonUp() override;
    virtual void OnRightButtonDown() override;
    virtual void OnRightButtonUp() override;
    virtual void OnMouseWheelForward() override;
    virtual void OnMouseWheelBackward() override;

    void setSimulationManager(SimulationManager* simManager);

private:

    SimulationManager* m_simManager;

};
}

#endif // ifndef imstkInteractorStyle_h
