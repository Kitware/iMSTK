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

#include "imstkGUICanvas.h"

namespace imstk
{
namespace GUIOverlay
{
Canvas::~Canvas()
{
    this->clear();
}

void
Canvas::clear()
{
    m_widgets.clear();
}

void
Canvas::render()
{
    if (!m_visible)
    {
        return;
    }

    for (int i = 0; i < m_widgets.getSize(); i++)
    {
        auto type = m_widgets[i]->getType();

        if (!m_widgets[i]->isVisible())
        {
            continue;
        }

        bool needsWindow = (type == Widget::Type::None);

        if (needsWindow)
        {
            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowBgAlpha(0.0);
            ImGui::Begin("", nullptr, ImGuiWindowFlags_NoInputs
                | ImGuiWindowFlags_NoTitleBar
                | ImGuiWindowFlags_NoResize);
        }

        m_widgets[i]->render(needsWindow);

        if (needsWindow)
        {
            ImGui::End();
        }
    }
}

void
Canvas::addWidget(std::shared_ptr<Widget> widget)
{
    m_widgets.addWidget(widget);
}

void
Canvas::removeWidget(std::string name)
{
    m_widgets.removeWidget(name);
}

std::shared_ptr<Widget>
Canvas::getWidget(std::string name)
{
    return m_widgets.getWidget(name);
}

void
Canvas::show()
{
    m_visible = true;
}

void
Canvas::hide()
{
    m_visible = false;
}
} // GUI
} // imstk
