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

#include "imstkLogger.h"

#include <vector>
#include <map>
#include <memory>

#include "imstkGUIWidget.h"

namespace imstk
{
namespace GUIOverlay
{
///
/// \class WidgetList
///
/// \brief Ordered list for Widget objects
///        The ordering matters, but we also want to have uniquely named
///        widgets.
///
class WidgetList
{
public:
    ///
    /// \brief Default constructor
    ///
    WidgetList();

    ///
    /// \brief Add widget
    ///
    void addWidget(std::shared_ptr<Widget> widget);

    ///
    /// \brief Remove widget
    ///
    bool removeWidget(const std::string& name);

    ///
    /// \brief Get widget
    ///
    std::shared_ptr<Widget> getWidget(const std::string& name);

    ///
    /// \brief Get number of widgets
    ///
    size_t getSize();

    ///
    /// \brief Clear widgets
    ///
    void clear();

    ///
    /// \brief Access operator
    ///
    std::shared_ptr<Widget> operator[](size_t index);

protected:
    std::vector<std::shared_ptr<Widget>> m_widgets;
};
} // GUI
} // imstk
