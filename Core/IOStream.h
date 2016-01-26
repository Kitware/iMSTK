// This file is part of the iMSTK project.
//
// Copyright (c) Kitware, Inc.
//
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

#ifndef CORE_IOSTREAM_H
#define CORE_IOSTREAM_H

#include "Core/Config.h"
#include "Core/CoreClass.h"

#include <iostream>
#include <string>
#include <unordered_map>

namespace imstk {

/// \brief I/O definitions
#define IMSTK_CONSOLE_INPUTBUFFER  512
#define IMSTK_WINDOW_MAXSTRINGSIZE 255
#define IMSTK_WINDOW_TOTALSTRINGS_ONWINDOW 100

namespace core {
    class Event;
    class EventHandler;
    class CameraEvent;
}

/// \brief I/O stream
class IOStream : public CoreClass
{
public:
    virtual IOStream& operator >>(std::string &p_string) = 0;
    virtual IOStream& operator <<(std::string p_string) = 0;
protected:
    std::shared_ptr<EventHandler> eventHanlder;
};

/// \brief  console stream; for printing text on the console
class ConsoleStream: public IOStream
{
    std::string inputBuffer;
public:
    ConsoleStream();
    /// \brief operator to print text
    virtual IOStream& operator <<(std::string p_string);

    /// \brief to  input from use
    virtual IOStream& operator >>(std::string &p_string);
};

/// \brief window string
struct WindowString
{
public:
    /// \brief  constructors
    WindowString();

    WindowString(std::string p_string);

    WindowString(std::string p_string, float p_x, float p_y);
    /// \brief operators for string
    WindowString &operator<<(std::string p_string);

    void operator =(WindowString &p_windowString);

public:
    /// \brief string
    std::string string;
    /// \brief position of string x,y
    float x, y;
};

struct WindowData
{
    bool enabled;
    WindowString windowString;
};
/// \brief window stream for putting window string on text
class WindowStream: public IOStream
{
public:
    virtual IOStream& operator <<(std::string p_string);
    virtual IOStream& operator >>(std::string &p_string);
};

/// \brief opengl window stream for putting text on the screen
class OpenGLWindowStream: public WindowStream
{
public:
    void init(int p_totalTexts);

    /// \brief enable/disable texts on display
    bool enabled;

    /// \brief  text color
    Color textColor;

    /// \brief constructors
    OpenGLWindowStream(int p_totalTexts = IMSTK_WINDOW_TOTALSTRINGS_ONWINDOW);

    /// \brief add text on window
    virtual int addText(const std::string &p_tag, const std::string &p_string);

    /// \brief add text on window
    bool addText(std::string p_tag, WindowString &p_string);

    /// \brief update the text with specificed tag(p_tag)
    bool updateText(std::string p_tag, std::string p_string);

    /// \brief add text on window with specified text handle
    bool updateText(int p_textHandle, std::string p_string);

    /// \brief remove text on window
    bool removeText(std::string p_tag);

    /// \brief  handle events
    virtual void handleEvent(std::shared_ptr<Event> /*p_event*/) override {}

protected:
    /// \brief  fonts
    //QFont font;
    /// \brief #of the total texts
    int totalTexts;
    /// \brief  window texts
    WindowData *windowTexts;
    std::unordered_map<std::string, int> tagMap;
    int currentIndex;
    /// \brief initial text position on window
    int initialTextPositionY;
    int initialTextPositionX;
    int lastTextPosition;
    /// \brief initialization routines
};
/// \brief window console
class WindowConsole: public OpenGLWindowStream
{
public:
    /// \brief window console constructor
    WindowConsole(int p_totalTexts = 5);
    /// \brief  return last entered entry
    std::string getLastEntry();

    /// \brief add text in the display
    virtual int addText(const std::string &p_tag, const std::string &p_string) override;

protected:
    /// \brief entered string on the console
    std::string enteredString;
    /// \brief window console position min, max points on the display
    float left;
    float bottom;
    float right;
    float top;
    /// \brief background color
    Color backGroundColor;
};

}

#endif
