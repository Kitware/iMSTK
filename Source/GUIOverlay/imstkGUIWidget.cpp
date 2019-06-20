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

#include "imstkGUIWidget.h"

namespace imstk
{
namespace GUIOverlay
{
Widget::Widget(std::string name, float x, float y)
{
    m_name = name;
    m_position[0] = x;
    m_position[1] = y;
}

Widget::~Widget()
{
}

std::string
Widget::getName()
{
    return m_name;
}

Widget::Type
Widget::getType()
{
    return m_type;
}

const Vec2f&
Widget::getPosition()
{
    return m_position;
}

void
Widget::setPosition(const float x, const float y)
{
    m_position[0] = x;
    m_position[1] = y;
}

void
Widget::setPosition(const Vec2f& position)
{
    m_position[0] = position[0];
    m_position[1] = position[1];
}

void
Widget::hide()
{
    m_visible = false;
}

void
Widget::show()
{
    m_visible = true;
}

const bool
Widget::isVisible()
{
    return m_visible;
}

void
Widget::render(const bool inWindow)
{
}
} // GUI
} // imstk
