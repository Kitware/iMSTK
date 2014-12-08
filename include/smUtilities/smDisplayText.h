/*
****************************************************
				  SOFMIS LICENSE

****************************************************

	\author:    <http:\\acor.rpi.edu>
				SOFMIS TEAM IN ALPHABATIC ORDER
				Anderson Maciel, Ph.D.
				Ganesh Sankaranarayanan, Ph.D.
				Sreekanth A Venkata
				Suvranu De, Ph.D.
				Tansel Halic
				Zhonghua Lu

	\author:    Module by Tansel Halic
				
				
	\version    1.0
	\date       04/2009
	\bug	    None yet
	\brief	    This Module implements text viewing mechanism for the framework. It can be any text and can be dispplayed on both console
				or on window text
				

*****************************************************
*/

#ifndef SMDISPLAYTEXT_H
#define SMDISPLAYTEXT_H
#include "smCore/smConfig.h"
#include "smCore/smCoreClass.h"
#include <iostream>

using namespace std;

/// \brief	display type
enum SMDISPLAY_STREAM{
	SMDISPLAY_CONSOLE,
	SMDISPLAY_WINDOW,
	SMDISPLAY_ERROR,
	SMDISPLAT_DEFAULT
};

/// \brief message type
struct smDisplayMessage{
	/// \brief text to display
	smChar text[SOFMIS_MAX_DISPLAYTEXT];
	/// \brief sender id. Id given my the framework
	smInt  senderId;
	/// \brief diplay output
	SMDISPLAY_STREAM outputStream;
	/// \brief x,y position
	int xPos;
	int yPos;
};

/// \brief display text class console, window, error log etc.
class smDisplayText:smCoreClass{
public:
	smDisplayText();
	void addText(smDisplayMessage message){
		switch(message.outputStream){
		case SMDISPLAY_CONSOLE:
			cout<<message.text<<endl;
		}
	}
};

#endif
