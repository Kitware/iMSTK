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

#include "imstkGUIWidgetList.h"

namespace imstk
{
namespace GUIOverlay
{
WidgetList::WidgetList()
{
}

void
WidgetList::addWidget(std::shared_ptr<Widget> widget)
{
    if (!this->getWidget(widget->getName()))
    {
        m_widgets.push_back(widget);
    }
}

bool
WidgetList::removeWidget(std::string name)
{
    int index = 0;

    for (auto widget : m_widgets)
    {
        if (widget->getName() == name)
        {
            m_widgets.erase(m_widgets.begin() + index);
            return true;
        }
        index++;
    }
    return false;
}

std::shared_ptr<Widget>
WidgetList::getWidget(std::string name)
{
    for (auto widget : m_widgets)
    {
        if (widget->getName() == name)
        {
            return widget;
        }
    }

    return nullptr;
}

std::shared_ptr<Widget>
WidgetList::operator[](size_t index)
{
    return m_widgets[index];
}

size_t
WidgetList::getSize()
{
    return m_widgets.size();
}

void
WidgetList::clear()
{
    m_widgets.clear();
}
} // GUI
} // imstk