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

#include <unordered_map>
#include <functional>

namespace imstk
{
class InteractorStyle;

/// Signature of custom function called in each even callback.
/// Return true to override base class behavior, or false to maintain it.
using EventHandlerFunction = std::function< bool(InteractorStyle* iStyle) >;

class InteractorStyle
{
public:
    InteractorStyle() {};

    virtual ~InteractorStyle(){};

protected:
    friend class Viewer;
    friend class VulkanViewer;

    /// Custom event handlers
    /// Return true to override default event slot
    std::unordered_map<char, EventHandlerFunction> m_onCharFunctionMap;
    EventHandlerFunction m_onMouseMoveFunction;
    EventHandlerFunction m_onLeftButtonDownFunction;
    EventHandlerFunction m_onLeftButtonUpFunction;
    EventHandlerFunction m_onMiddleButtonDownFunction;
    EventHandlerFunction m_onMiddleButtonUpFunction;
    EventHandlerFunction m_onRightButtonDownFunction;
    EventHandlerFunction m_onRightButtonUpFunction;
    EventHandlerFunction m_onMouseWheelForwardFunction;
    EventHandlerFunction m_onMouseWheelBackwardFunction;
    EventHandlerFunction m_onTimerFunction;
};
}

#endif