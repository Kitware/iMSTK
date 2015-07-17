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

#ifndef SMIOSTREAM_H
#define SMIOSTREAM_H

#include "smConfig.h"
#include "smCoreClass.h"
#include "smSDK.h"

#include <iostream>
#include <string>
#include <unordered_map>

/// \brief I/O definitions
#define SM_CONSOLE_INPUTBUFFER  512
#define SM_WINDOW_MAXSTRINGSIZE 255
#define SM_WINDOW_TOTALSTRINGS_ONWINDOW 100

namespace smtk {
namespace Event {
    class smEvent;
    class smEventHandler;
    class smCameraEvent;
    }
}

/// \brief I/O stream
class smIOStream : public smCoreClass
{
public:
    virtual smIOStream& operator >>(smString &p_string) = 0;
    virtual smIOStream& operator <<(smString p_string) = 0;
protected:
    std::shared_ptr<smtk::Event::smEventHandler> eventHanlder;
};

/// \brief  console stream; for printing text on the console
class smConsoleStream: public smIOStream
{
    smString inputBuffer;
public:
    smConsoleStream();
    /// \brief operator to print text
    virtual smIOStream& operator <<(smString p_string);

    /// \brief to  input from use
    virtual smIOStream& operator >>(smString &p_string);
};

/// \brief window string
struct smWindowString
{
public:
    /// \brief  constructors
    smWindowString();

    smWindowString(smString p_string);

    smWindowString(smString p_string, smFloat p_x, smFloat p_y);
    /// \brief operators for string
    smWindowString &operator<<(smString p_string);

    void operator =(smWindowString &p_windowString);

public:
    /// \brief string
    smString string;
    /// \brief position of string x,y
    smFloat x, y;
};

struct smWindowData
{
    smBool enabled;
    smWindowString windowString;
};
/// \brief window stream for putting window string on text
class smWindowStream: public smIOStream
{
public:
    virtual smIOStream& operator <<(smString p_string);
    virtual smIOStream& operator >>(smString &p_string);
};

/// \brief opengl window stream for putting text on the screen
class smOpenGLWindowStream: public smWindowStream
{
public:
    void init(smInt p_totalTexts);

    /// \brief enable/disable texts on display
    smBool enabled;

    /// \brief  text color
    smColor textColor;

    /// \brief constructors
    smOpenGLWindowStream(smInt p_totalTexts = SM_WINDOW_TOTALSTRINGS_ONWINDOW);

    /// \brief add text on window
    virtual smInt addText(const smString &p_tag, const smString &p_string);

    /// \brief add text on window
    bool addText(smString p_tag, smWindowString &p_string);

    /// \brief update the text with specificed tag(p_tag)
    bool updateText(smString p_tag, smString p_string);

    /// \brief add text on window with specified text handle
    bool updateText(smInt p_textHandle, smString p_string);

    /// \brief remove text on window
    bool removeText(smString p_tag);

    /// \brief  handle events
    virtual void handleEvent(std::shared_ptr<smtk::Event::smEvent> /*p_event*/) override {}

protected:
    /// \brief  fonts
    //QFont font;
    /// \brief #of the total texts
    smInt totalTexts;
    /// \brief  window texts
    smWindowData *windowTexts;
    std::unordered_map<smString, smInt> tagMap;
    smInt currentIndex;
    /// \brief initial text position on window
    smInt initialTextPositionY;
    smInt initialTextPositionX;
    smInt lastTextPosition;
    /// \brief initialization routines
};
/// \brief window console
class smWindowConsole: public smOpenGLWindowStream
{
public:
    /// \brief window console constructor
    smWindowConsole(smInt p_totalTexts = 5);
    /// \brief  return last entered entry
    smString getLastEntry();

    /// \brief add text in the display
    virtual smInt addText(const smString &p_tag, const smString &p_string) override;

protected:
    /// \brief entered string on the console
    smString enteredString;
    /// \brief window console position min, max points on the display
    smFloat left;
    smFloat bottom;
    smFloat right;
    smFloat top;
    /// \brief background color
    smColor backGroundColor;
};

#endif
