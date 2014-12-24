/*****************************************************
                  SIMMEDTK LICENSE
****************************************************

*****************************************************/

#ifndef SMDISPLAYTEXT_H
#define SMDISPLAYTEXT_H
#include "smCore/smConfig.h"
#include "smCore/smCoreClass.h"
#include <iostream>

using namespace std;

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
    smChar text[SIMMEDTK_MAX_DISPLAYTEXT];
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
    void addText(smDisplayMessage message)
    {
        switch (message.outputStream)
        {
        case SMDISPLAY_CONSOLE:
            cout << message.text << endl;
        }
    }
};

#endif
