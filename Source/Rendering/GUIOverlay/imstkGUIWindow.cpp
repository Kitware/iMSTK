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

#include "imstkGUIWindow.h"
#include "imgui.h"

namespace imstk
{
namespace GUIOverlay
{
Window::Window(const std::string& name,
               const std::string& title,
               const float        width,
               const float        height,
               const float        x,
               const float        y) :
    Widget(name, x, y),
    m_title(title),
    m_width(width),
    m_height(height)
{
    m_type = Widget::Type::Window;
}

Window::~Window()
{
    this->clear();
}

void
Window::clear()
{
    m_widgets.clear();
}

#pragma warning(push)
#pragma warning( disable : 4100 )
/// \todo clear C4100
void
Window::render(const bool inWindow)
#pragma warning(pop)
{
    ImGui::SetNextWindowPos(ImVec2(m_position[0], m_position[1]));
    ImGui::SetNextWindowSize(ImVec2(m_width, m_height));
    ImGui::SetNextWindowBgAlpha(0.5f);
    ImGui::Begin(m_title.c_str(), nullptr, ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoCollapse
        | ImGuiWindowFlags_NoBringToFrontOnFocus);
    for (int i = 0; i < m_widgets.getSize(); i++)
    {
        if (!m_widgets[i]->isVisible())
        {
            continue;
        }
        m_widgets[i]->render(true);
    }
    ImGui::End();
}

void
Window::addWidget(std::shared_ptr<Widget> widget)
{
    m_widgets.addWidget(widget);
}

std::shared_ptr<Widget>
Window::getWidget(std::string name)
{
    return m_widgets.getWidget(name);
}
} // GUI
} // imstk
