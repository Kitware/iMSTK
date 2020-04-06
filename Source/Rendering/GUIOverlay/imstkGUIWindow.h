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

#include "imstkLogUtility.h"

#include <string>
#include <map>
#include <memory>

#include "imstkGUIWidget.h"
#include "imstkGUIWidgetList.h"

namespace imstk
{
namespace GUIOverlay
{
///
/// \class PointSet
///
/// \brief Base class for all geometries represented by discrete points and elements
///
class Window : public Widget
{
public:

    ///
    /// \brief Protected constructor
    ///
    explicit Window(const std::string& name,
                    const std::string& title = "",
                    const float        width  = 100,
                    const float        height = 50,
                    const float        x      = 0,
                    const float        y      = 0);

    ///
    /// \brief Destructor
    ///
    ~Window();

    ///
    /// \brief Clears all the widgets
    ///
    void clear();

    ///
    /// \brief Render
    ///
    virtual void render(const bool inWindow) override;

    ///
    /// \brief Add widget
    ///
    void addWidget(std::shared_ptr<Widget> widget);

    ///
    /// \brief Get widget
    ///
    std::shared_ptr<Widget> getWidget(std::string name);

protected:
    Window() = delete;

    std::string m_title = "";
    float       m_width;
    float       m_height;
    WidgetList  m_widgets;
};
} // GUI
} // imstk
