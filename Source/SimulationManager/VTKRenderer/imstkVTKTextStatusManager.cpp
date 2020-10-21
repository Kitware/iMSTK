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

#include "imstkVTKTextStatusManager.h"
#include "imstkVTKInteractorStyle.h"
#include "imstkVTKViewer.h"
#include "imstkLogger.h"

#include <vtkRenderWindow.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>

namespace imstk
{
VTKTextStatusManager::VTKTextStatusManager()
{
    for (int i = 0; i < static_cast<int>(StatusType::NumStatusTypes); ++i)
    {
        m_StatusActors[i] = vtkTextActor::New();
        m_StatusActors[i]->SetVisibility(false);
        setStatusFontSize(static_cast<StatusType>(i), 60);
        setStatusDisplayCorner(static_cast<StatusType>(i), static_cast<DisplayCorner>(i));
    }
}

VTKTextStatusManager::~VTKTextStatusManager()
{
    for (int i = 0; i < static_cast<int>(StatusType::NumStatusTypes); ++i)
    {
        m_StatusActors[i]->Delete();
    }
}

vtkTextActor*
VTKTextStatusManager::getTextActor(const int i)
{
    if (i < 0 || i >= static_cast<int>(StatusType::NumStatusTypes))
    {
        LOG(WARNING) << "Invalid text actor index";
        return m_StatusActors[0];
    }
    return m_StatusActors[i];
}

void
VTKTextStatusManager::setStatusVisibility(const StatusType type, const bool bVisible)
{
    m_StatusActors[static_cast<int>(type)]->SetVisibility(bVisible);
}

bool
VTKTextStatusManager::getStatusVisibility(const StatusType type)
{
    return m_StatusActors[static_cast<int>(type)]->GetVisibility();
}

void
VTKTextStatusManager::setStatusFontSize(const StatusType type, const int fontSize)
{
    m_StatusFontSizes[static_cast<int>(type)] = fontSize; // Store font size to compute and adjust text location later
    m_StatusActors[static_cast<int>(type)]->GetTextProperty()->SetFontSize(fontSize);
}

void
VTKTextStatusManager::setStatusFontColor(const StatusType type, const Color color)
{
    m_StatusActors[static_cast<int>(type)]->GetTextProperty()->SetColor(color.r, color.g, color.b);
}

void
VTKTextStatusManager::setStatusDisplayCorner(const StatusType type, const DisplayCorner corner)
{
    m_StatusDisplayCorners[static_cast<int>(type)] = corner;
}

void
VTKTextStatusManager::setFPS(const double visualFPS, const double physicsFPS)
{
    std::string fpsVisualStr = "V: " + std::to_string(static_cast<int>(visualFPS));
    std::string fpsPhysicalStr;

    if (std::isnan(physicsFPS) || std::isinf(physicsFPS))
    {
        fpsPhysicalStr = "P: Inf";
    }
    else if (physicsFPS < 0)
    {
        fpsPhysicalStr = "P: PAUSED";
    }
    else
    {
        char buff[32];
        (physicsFPS < 4.0) ? IMSTK_SPRINT(buff, "%.2f", physicsFPS) :
        IMSTK_SPRINT(buff, "%d", static_cast<int>(physicsFPS));
        fpsPhysicalStr = "P: " + std::string(buff);
    }

    std::string fpsString = fpsVisualStr + std::string(" | ") + fpsPhysicalStr;
    auto        fpsStatusCoordinate = computeStatusLocation(m_StatusDisplayCorners[static_cast<int>(StatusType::FPS)],
                                                     m_StatusFontSizes[static_cast<int>(StatusType::FPS)],
                                                     fpsString);
    m_StatusActors[static_cast<int>(StatusType::FPS)]->SetDisplayPosition(fpsStatusCoordinate[0], fpsStatusCoordinate[1]);
    m_StatusActors[static_cast<int>(StatusType::FPS)]->SetInput((fpsString).c_str());
}

void
VTKTextStatusManager::setCustomStatus(const std::string& status)
{
    auto customStatusCoordinate = computeStatusLocation(m_StatusDisplayCorners[static_cast<int>(StatusType::Custom)],
                                                        m_StatusFontSizes[static_cast<int>(StatusType::Custom)],
                                                        status);
    m_StatusActors[static_cast<int>(StatusType::Custom)]->SetInput(status.c_str());
    m_StatusActors[static_cast<int>(StatusType::Custom)]->SetDisplayPosition(customStatusCoordinate[0], customStatusCoordinate[1]);
    m_StatusActors[static_cast<int>(StatusType::Custom)]->SetVisibility(true);
}

void
VTKTextStatusManager::clearCustomStatus()
{
    m_StatusActors[static_cast<int>(StatusType::Custom)]->SetInput("");
    m_StatusActors[static_cast<int>(StatusType::Custom)]->SetVisibility(false);
}

std::array<int, 2>
VTKTextStatusManager::computeStatusLocation(const DisplayCorner corner, const int fontSize, const std::string& text)
{
    const int* windowSize   = m_viewer->getVtkRenderWindow()->GetSize();
    const auto wWidth       = windowSize[0];
    const auto wHeight      = windowSize[1];
    const int  numLines     = static_cast<int>(std::count(text.begin(), text.end(), '\n')) + 1;
    int        maxLineWidth = 0;

    if (numLines <= 1)
    {
        maxLineWidth = static_cast<int>(text.length());
    }
    else
    {
        std::stringstream ss(text);
        std::string       line;
        while (std::getline(ss, line, '\n'))
        {
            int lineWidth = static_cast<int>(line.length());
            if (maxLineWidth < lineWidth)
            {
                maxLineWidth = lineWidth;
            }
        }
    }

    static const int   padding = 10; // Add an extra space to the corner
    std::array<int, 2> location;     // x, y
    location[0] = padding;
    location[1] = padding;

    switch (corner)
    {
    case DisplayCorner::LowerLeft:
        // Use the default value
        break;

    case DisplayCorner::UpperLeft:
        location[1] = wHeight - padding - fontSize * numLines;
        break;

    case DisplayCorner::UpperRight:
        location[0] = wWidth - padding - (maxLineWidth + 2) * fontSize / 2;
        location[1] = wHeight - padding - fontSize * numLines;
        break;

    case DisplayCorner::LowerRight:
        location[0] = wWidth - padding - (maxLineWidth + 2) * fontSize / 2;
        break;
    }

    return location;
}
} // end namespace imstk
