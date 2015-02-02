/*=========================================================================
 * Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
 *                        Rensselaer Polytechnic Institute
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 /=========================================================================
 
 /**
  *  \brief
  *  \details
  *  \author
  *  \author
  *  \copyright Apache License, Version 2.0.
  */
 
#ifndef SMIOSTREAM_H
#define SMIOSTREAM_H

#include <QString>
#include <iostream>
#include <QFont>
#include <QHash>

#include "smCore/smConfig.h"
#include "smCore/smCoreClass.h"
#include "smRendering/smViewer.h"
#include "smUtilities/smGLUtils.h"
#include "smCore/smEventData.h"
#include "smCore/smEventHandler.h"
#include "smCore/smSDK.h"

/// \brief I/O definitions
#define SM_CONSOLE_INPUTBUFFER  512
#define SM_WINDOW_MAXSTRINGSIZE 255
#define SM_WINDOW_TOTALSTRINGS_ONWINDOW 100
#define SM_WINDOWHASH_INDEX(X) qHash(X)%SM_WINDOW_TOTALSTRINGS_ONWINDOW

/// \brief copy string
using namespace std;
#define COPY_STRING( p_stringDst, p_stringSrc) {\
        smInt ijk;\
        for(ijk=0;ijk<p_stringSrc.size();ijk++)\
            p_stringDst[ijk]=p_stringSrc[ijk];\
        p_stringDst.truncate(ijk);\
        }\
 
/// \brief I/O stream
class smIOStream: public smCoreClass
{
public:
    virtual smIOStream& operator >>(QString &p_string) = 0;
    virtual smIOStream& operator <<(QString p_string) = 0;
};

/// \brief  console stream; for printing text on the console
class smConsoleStream: public smIOStream
{
    smChar inputBuffer[SM_CONSOLE_INPUTBUFFER];
public:
    smConsoleStream()
    {
    }
    /// \brief operator to print text
    virtual smIOStream& operator <<(QString p_string)
    {
        cout << qPrintable(p_string);
        return *this;
    }

    /// \brief to  input from use
    virtual smIOStream& operator >>(QString &p_string)
    {
        cin.get(inputBuffer, SM_CONSOLE_INPUTBUFFER - 1);
        inputBuffer[SM_CONSOLE_INPUTBUFFER - 1] = '\0';
        p_string = p_string.fromLocal8Bit(inputBuffer);
        return *this;
    }
};

/// \brief window string
struct smWindowString
{
public:
    /// \brief string
    QString string;
    /// \brief position of string x,y
    smFloat x, y;
    /// \brief  constructors
    smWindowString()
    {
        x = 0;
        y = 0;
        string = "";
        string.reserve(SM_WINDOW_MAXSTRINGSIZE);
    }

    smWindowString(QString p_string)
    {
        string = p_string;
    }

    smWindowString(QString p_string, smFloat p_x, smFloat p_y)
    {
        string = p_string.toAscii();
        x = p_x;
        y = p_y;
    }
    /// \brief operators for string
    smWindowString &operator<<(QString p_string)
    {
        string = p_string;
        return *this;
    }

    void operator =(smWindowString &p_windowString)
    {
        string.clear();
        COPY_STRING(string, p_windowString.string);
        x = p_windowString.x;
        y = p_windowString.y;
    }
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
    virtual smIOStream& operator <<(QString p_string)
    {
        return *this;
    }
    virtual smIOStream& operator >>(QString &p_string)
    {
        return *this;
    }
};

/// \brief opengl window stream for putting text on the screen
class smOpenGLWindowStream: public smWindowStream
{
protected:
    /// \brief  fonts
    QFont font;
    /// \brief #of the total texts
    smInt totalTexts;
    /// \brief  window texts
    smWindowData *windowTexts;
    smInt *tagMap;
    smInt currentIndex;
    /// \brief initial text position on window
    smInt initialTextPositionY;
    smInt initialTextPositionX;
    smInt lastTextPosition;
    /// \brief initialization routines
    void init(smInt p_totalTexts)
    {
        textColor.setValue(1.0, 1.0, 1.0, 1.0);
        totalTexts = p_totalTexts;
        windowTexts = new smWindowData[totalTexts];
        tagMap = new smInt[totalTexts];
        drawOrder = SIMMEDTK_DRAW_AFTEROBJECTS;

        for (smInt i = 0; i < totalTexts; i++)
        {
            windowTexts[i].enabled = false;
        }

        enabled = true;
        currentIndex = 0;
        initialTextPositionX = 0.0;
        initialTextPositionY = font.pointSize(); //+font.pointSize()/2.0;
        lastTextPosition = 0;
    }

public:
    /// \brief enable/disable texts on display
    smBool enabled;
    /// \brief  text color
    smColor textColor;

    /// \brief constructors
    smOpenGLWindowStream(smInt p_totalTexts = SM_WINDOW_TOTALSTRINGS_ONWINDOW)
    {
        font.setPointSize(10.0);
        init(p_totalTexts);
    }

    smOpenGLWindowStream(QFont p_font, smInt p_totalTexts = SM_WINDOW_TOTALSTRINGS_ONWINDOW)
    {
        font = p_font;
        init(p_totalTexts);
    }

    smOpenGLWindowStream(smInt p_fontSize, smInt p_totalTexts = SM_WINDOW_TOTALSTRINGS_ONWINDOW)
    {
        font.setPointSize(p_fontSize);
        init(p_totalTexts);
    }
    /// \brief add text on window
    virtual smInt addText(const QString &p_tag, const QString &p_string)
    {
        smWindowString string;
        string.string = p_string;
        string.x = 0;
        string.y = lastTextPosition;
        lastTextPosition += font.pointSize() + font.pointSize() / 2.0;
        tagMap[SM_WINDOWHASH_INDEX(p_tag)] = currentIndex;
        windowTexts[currentIndex].enabled = true;
        windowTexts[currentIndex].windowString = string;
        currentIndex = (currentIndex + 1) % totalTexts;
        return currentIndex;
    }
    /// \brief add text on window
    bool addText(QString p_tag, smWindowString &p_string)
    {
        if (p_string.string.size() > SM_WINDOW_MAXSTRINGSIZE)
        {
            return false;
        }

        currentIndex = (currentIndex + 1) % totalTexts;
        tagMap[SM_WINDOWHASH_INDEX(p_tag)] = currentIndex;
        windowTexts[currentIndex].windowString = p_string;
        windowTexts[currentIndex].enabled = true;
        return true;
    }
    /// \brief update the text with specificed tag(p_tag)
    bool updateText(QString p_tag, QString p_string)
    {
        smInt index = -1;

        if (p_string.size() > SM_WINDOW_MAXSTRINGSIZE)
        {
            return false;
        }

        index = tagMap[SM_WINDOWHASH_INDEX(p_tag)];

        if (index >= 0)
            COPY_STRING(windowTexts[index].windowString.string, p_string)
            else
            {
                return false;
            }

        return true;
    }
    /// \brief add text on window with specified text handle
    bool updateText(smInt p_textHandle, QString p_string)
    {
        smInt index = p_textHandle;

        if (p_string.size() > SM_WINDOW_MAXSTRINGSIZE)
        {
            return false;
        }

        if (index >= 0)
            COPY_STRING(windowTexts[index].windowString.string, p_string)
            else
            {
                return false;
            }

        return true;
    }
    /// \brief remove text on window
    bool removeText(QString p_tag)
    {
        smInt index = tagMap[SM_WINDOWHASH_INDEX(p_tag)];
        windowTexts[index].enabled = false;
        return true;
    }
    /// \brief draw text on window
    virtual void draw(smDrawParam p_params)
    {
        smViewer *viewer = (smViewer *)p_params.rendererObject;
        glColor3fv(smColor::colorWhite.toGLColor());

        /*
        //This needs to be replaced by some opengl text api in the future
        for (smInt i = 0; i < totalTexts; i++)
        {
            if (windowTexts[i].enabled)
            {
                viewer->drawText(windowTexts[i].windowString.x + initialTextPositionX, windowTexts[i].windowString.y + initialTextPositionY, windowTexts[i].windowString.string, font);
            }

        }
        */
    }
};
/// \brief window console
class smWindowConsole: public smOpenGLWindowStream, public smEventHandler
{
protected:
    /// \brief entered string on the console
    QString enteredString;
    /// \brief window console position min, max points on the display
    smFloat left;
    smFloat bottom;
    smFloat right;
    smFloat top;
    /// \brief background color
    smColor backGroundColor;
public:
    /// \brief window console constructor
    smWindowConsole(smInt p_totalTexts = 5)
    {
        init(p_totalTexts);
        backGroundColor.setValue(1.0, 1.0, 1.0, 0.15);
        smSDK::getInstance()->getEventDispatcher()->registerEventHandler(this, SIMMEDTK_EVENTTYPE_KEYBOARD);
        left = 0.0;
        bottom = 0.0;
        right = 1.0;
        top = 0.15;
    }
    /// \brief  return last entered entry
    QString getLastEntry()
    {
        return windowTexts[currentIndex].windowString.string;
    }

    /// \brief add text in the display
    virtual smInt addText(QString p_tag, QString &p_string)
    {
        smInt traverseIndex;
        smInt counter = 0;
        smWindowString string;
        string.string = p_string;
        windowTexts[currentIndex].enabled = true;
        windowTexts[currentIndex].windowString = string;
        tagMap[SM_WINDOWHASH_INDEX(p_tag)] = currentIndex;

        for (smInt i = currentIndex, counter = 0; counter < totalTexts; i--, counter++)
        {
            if (i < 0)
            {
                i += totalTexts;
            }

            traverseIndex = i % totalTexts;
            windowTexts[traverseIndex].windowString.x = 0.0;
            windowTexts[traverseIndex].windowString.y = (font.pointSize() * (totalTexts - counter));
        }

        currentIndex = (currentIndex + 1) % totalTexts;
        return currentIndex;
    }
    /// \brief  draw console
    virtual void draw(smDrawParam p_params)
    {
        smInt drawIndex = 0;
        smGLUtils::drawQuadOnScreen(backGroundColor, left, bottom, right, top);
        smViewer *viewer = (smViewer *)p_params.rendererObject;

        initialTextPositionY = viewer->height() - viewer->height() * top + font.pointSize();
        glColor3fv(smColor::colorWhite.toGLColor());

        /*
        for (smInt i = currentIndex; i < currentIndex + totalTexts; i++)
        {
            drawIndex = i % totalTexts;

            if (windowTexts[drawIndex].enabled)
            {
                viewer->drawText(windowTexts[drawIndex].windowString.x + initialTextPositionX, windowTexts[drawIndex].windowString.y + initialTextPositionY, windowTexts[drawIndex].windowString.string, font);
            }
        }

        viewer->drawText(0, viewer->height() - 10, ">>>" + enteredString, font);
        */
    }
    /// \brief  handle events
    void handleEvent(smEvent *p_event)
    {
        smKeyboardEventData *keyBoardData;

        switch (p_event->eventType.eventTypeCode)
        {
        case SIMMEDTK_EVENTTYPE_KEYBOARD:
            keyBoardData = (smKeyboardEventData*)p_event->data;

            if (keyBoardData->keyBoardKey == Qt::Key_Return || keyBoardData->keyBoardKey == Qt::Key_Enter)
            {
                this->addText("line1", enteredString);
                enteredString = "";
            }
            else if (keyBoardData->keyBoardKey == Qt::Key_Delete || keyBoardData->keyBoardKey == Qt::Key_Backspace)
            {
                enteredString.chop(1);
            }
            else
            {
                enteredString.append(QChar(keyBoardData->keyBoardKey));
            }

            break;
        }
    }
};

#endif
