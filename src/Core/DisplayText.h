// This file is part of the SimMedTK project.
// Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
//                        Rensselaer Polytechnic Institute
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//---------------------------------------------------------------------------
//
// Authors:
//
// Contact:
//---------------------------------------------------------------------------
#ifndef SMDISPLAYTEXT_H
#define SMDISPLAYTEXT_H

// SimMedTK includes
#include "Config.h"
#include "CoreClass.h"

/// \brief  display type
enum SMDISPLAY_STREAM
{
    SMDISPLAY_CONSOLE,
    SMDISPLAY_WINDOW,
    SMDISPLAY_ERROR,
    SMDISPLAT_DEFAULT
};

/// \brief message type
struct smDisplayMessage
{
    /// \brief text to display
    smString text;
    /// \brief sender id. Id given my the framework
    smInt  senderId;
    /// \brief diplay output
    SMDISPLAY_STREAM outputStream;
    /// \brief x,y position
    int xPos;
    int yPos;
};

/// \brief display text class console, window, error log etc.
class smDisplayText: smCoreClass
{
public:
    smDisplayText();
    void addText(smDisplayMessage message);
};

#endif
