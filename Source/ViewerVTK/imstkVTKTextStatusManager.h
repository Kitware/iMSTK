/*=========================================================================

    Library: iMSTK

    Copyright (c) Kitware

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

#include "imstkColor.h"
#include "imstkMath.h"

#include <array>

class vtkTextActor;

// Cross-platform sprintf
#if defined(_WIN32) || defined(_WIN64)
#   define IMSTK_SPRINT sprintf_s
#else
#   define IMSTK_SPRINT sprintf
#endif

namespace imstk
{
class VTKViewer;

///
/// \class VTKTextStatusManager
///
/// \brief The VTKTextStatusManager class
///
class VTKTextStatusManager
{
public:
    ///
    /// \brief The StatusType enum
    ///
    enum class StatusType
    {
        FPS = 0,
        Custom,
        NumStatusTypes
    };

    ///
    /// \brief The TextLocation enum
    ///
    enum class DisplayCorner
    {
        LowerLeft,
        UpperLeft,
        UpperRight,
        LowerRight,
        CenterCenter
    };

    VTKTextStatusManager();
    virtual ~VTKTextStatusManager();

    ///
    /// \brief Return the text actor at index i
    ///
    vtkTextActor* getTextActor(const int i);

    ///
    /// \brief Set the visibility of the text status
    ///
    void setStatusVisibility(const StatusType type, const bool bVisible);

    ///
    /// \brief Get the visibility of the text status
    ///
    bool getStatusVisibility(const StatusType type);

    ///
    /// \brief Set the font size for the status
    ///
    void setStatusFontSize(const StatusType type, const int fontSize);

    ///
    /// \brief Set the font color for the status
    ///
    void setStatusFontColor(const StatusType type, const Color color);

    ///
    /// \brief Set the location (corner) of the custom status
    ///
    void setStatusDisplayCorner(const StatusType type, const DisplayCorner corner);

    ///
    /// \brief Set the FPS numbers
    /// \param visualFPS The visual FPS, must be non-negative
    /// \param physicalFPS The physcal FPS, negative value means paused
    ///
    void setFPS(const double visualFPS, const double physicsFPS);

    ///
    /// \brief Set the window size
    ///
    void setWindowSize(VTKViewer* viewer) { m_viewer = viewer; }

    ///
    /// \brief Set custom status text
    /// The string parameter should be passed by value, not const reference
    ///
    void setCustomStatus(const std::string& status);

    ///
    /// \brief Clear custom status text (and hide it)
    ///
    void clearCustomStatus();

private:
    ///
    /// \brief Compute the location for text, based on the current TextLocation enum and the length of the given text
    ///
    std::array<int, 2> computeStatusLocation(const DisplayCorner corner, const int fontSize, const std::string& text);

    vtkTextActor* m_StatusActors[static_cast<int>(StatusType::NumStatusTypes)];         ///< Statuses to display
    DisplayCorner m_StatusDisplayCorners[static_cast<int>(StatusType::NumStatusTypes)]; ///< Location (corner) of the statuses
    int m_StatusFontSizes[static_cast<int>(StatusType::NumStatusTypes)];                ///< Size of the statuses

    VTKViewer* m_viewer;                                                                // Used for dimensions
};
} // namespace imstk
