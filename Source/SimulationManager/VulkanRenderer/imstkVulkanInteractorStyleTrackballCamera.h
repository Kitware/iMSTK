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

#ifndef imstkVulkanInteractorStyleTrackballCamera_h
#define imstkVulkanInteractorStyleTrackballCamera_h

#include "GLFW/glfw3.h"

#include <iostream>
#include <unordered_map>
#include <functional>

namespace imstk
{
class VulkanViewer;
class SimulationManager;

class VulkanInteractorStyleTrackballCamera
{
public:
    VulkanInteractorStyleTrackballCamera();
    ~VulkanInteractorStyleTrackballCamera(){};

    void setWindow(GLFWwindow * window, VulkanViewer * viewer);

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
    void OnWindowResize(int width, int height);

    // Dispatch function
    static void OnCharInterface(GLFWwindow * window, int a, int b, int c, int d);
    static void OnMouseButtonInterface(GLFWwindow * window, int a, int b, int c);
    static void OnMouseMoveInterface(GLFWwindow * window, double x, double y);
    static void OnMouseWheelInterface(GLFWwindow * window, double x, double y);

    static void OnWindowResizeInterface(GLFWwindow * window, int width, int height);
    static void OnFramebuffersResizeInterface(GLFWwindow * window, int width, int height);
    static void OnFrame();

protected:
    friend class VulkanViewer;

    GLFWwindow * m_window;
    SimulationManager * m_simManager;
    VulkanViewer * m_viewer;

    double distance(double x, double y);

    // States
    enum
    {
        LEFT_MOUSE_DOWN = 0x1,
        MIDDLE_MOUSE_DOWN = 0x2,
        RIGHT_MOUSE_DOWN = 0x4
    };

    double m_lastMouseX = 0;
    double m_lastMouseY = 0;

    double m_mouseX = 0;
    double m_mouseY = 0;

    unsigned int m_state = 0;
};
}

#endif