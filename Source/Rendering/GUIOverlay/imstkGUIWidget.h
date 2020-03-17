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

#pragma once

#include <string>
#include "imstkMath.h"

namespace imstk
{
namespace GUIOverlay
{
///
/// \class Widget
///
/// \brief Base class for all GUI elements
///
class Widget
{
public:
    ///
    /// \enum Type
    ///
    /// \brief Quick identifier for GUI element type
    ///
    enum class Type
    {
        None,
        Window,
        Shape,
        Text
    };

    ///
    /// \brief Protected constructor
    ///
    Widget(std::string name, float x = 0.0f, float y = 0.0f);

    ///
    /// \brief Destructor
    ///
    ~Widget();

    ///
    /// \brief Get name
    ///
    std::string getName();

    ///
    /// \brief Get type
    ///
    Widget::Type getType();

    ///
    /// \brief Get position
    ///
    const Vec2f& getPosition();

    ///
    /// \brief Set position
    ///
    void setPosition(const float x, const float y);
    void setPosition(const Vec2f& position);

    ///
    /// \brief Hide
    ///
    void hide();

    ///
    /// \brief Show
    ///
    void show();

    ///
    /// \brief Get visibility
    ///
    const bool isVisible();

    ///
    /// \brief Render
    ///
    virtual void render(const bool inWindow = false);

protected:
    Widget() = delete;

    std::string  m_name;
    Vec2f        m_position;
    Widget::Type m_type = Widget::Type::None;
    bool m_visible      = true;
};
} // GUI
} // imstk
