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

#include "imgui.h"

#include <map>
#include <string>
#include <memory>

#include "imstkGUIWidget.h"
#include "imstkGUIWidgetList.h"

namespace imstk
{
namespace GUIOverlay
{
///
/// \class Canvas
///
/// \brief Base class for GUI
///
class Canvas
{
public:

    ///
    /// \brief Protected constructor
    ///
    Canvas() {};

    ///
    /// \brief Destructor
    ///
    ~Canvas();

    ///
    /// \brief Clears all the widgets
    ///
    void clear();

    ///
    /// \brief Render the widgets
    ///
    void render();

    ///
    /// \brief Add widget
    ///
    void addWidget(std::shared_ptr<Widget> widget);

    ///
    /// \brief Remove widget
    ///
    void removeWidget(std::string name);

    ///
    /// \brief Get widget
    ///
    std::shared_ptr<Widget> getWidget(std::string name);

    ///
    /// \brief Show canvas
    ///
    void show();

    ///
    /// \brief Hide canvas
    ///
    void hide();

protected:

    WidgetList m_widgets;
    bool m_visible = true;
};
} // GUI
} // imstk
