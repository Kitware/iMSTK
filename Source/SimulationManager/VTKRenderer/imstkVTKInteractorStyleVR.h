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

#include "imstkInteractorStyle.h"
#include "imstkMath.h"
#include "imstkSignal.h"

#include <vtkInteractorStyle3D.h>

namespace imstk
{
class SimulationManager;
}

///
/// \brief Interactor style for VR
///
class vtkInteractorStyleVR : public vtkInteractorStyle3D, public imstk::InteractorStyle
{
public:
    static vtkInteractorStyleVR* New();
    vtkTypeMacro(vtkInteractorStyleVR, vtkInteractorStyle3D);

    void OnMove3D(vtkEventData* edata) override;
    void OnButton3D(vtkEventData* edata) override;
    void OnTimer() override;

public:
    imstk::expiremental::Signal<imstk::Vec3d, imstk::Quatd> leftControllerPoseChanged;
    imstk::expiremental::Signal<imstk::Vec3d, imstk::Quatd> rightControllerPoseChanged;
    imstk::expiremental::Signal<imstk::Vec3d, imstk::Quatd> hmdPoseChanged;

    imstk::SimulationManager* m_simManager;     ///> SimulationManager owning the current simulation being interacted with
};