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

#ifndef imstkVulkanInteractorStyle_h
#define imstkVulkanInteractorStyle_h

#include "GLFW/glfw3.h"

#include "imstkVulkanInteractorStyleTrackballCamera.h"
#include "imstkInteractorStyle.h"

#include <iostream>
#include <unordered_map>
#include <functional>
#include <cctype>

namespace imstk
{
class VulkanInteractorStyle;
class VulkanViewer;

using VulkanBaseInteractorStyle = VulkanInteractorStyleTrackballCamera;

class VulkanInteractorStyle : public VulkanBaseInteractorStyle, public InteractorStyle
{
public:
    VulkanInteractorStyle();
    virtual ~VulkanInteractorStyle(){};

    virtual void OnTimer();
    virtual void OnChar(int keyID, int type);
    virtual void OnMouseMove(double x, double y);
    virtual void OnLeftButtonDown();
    virtual void OnLeftButtonUp();
    virtual void OnMiddleButtonDown();
    virtual void OnMiddleButtonUp();
    virtual void OnRightButtonDown();
    virtual void OnRightButtonUp();
    virtual void OnMouseWheelForward(double y);
    virtual void OnMouseWheelBackward(double y);

private:
    friend class VulkanViewer;
};
}

#endif