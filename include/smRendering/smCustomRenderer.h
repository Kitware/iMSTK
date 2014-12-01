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
	\date       05/2009
	\bug	    None yet
	\brief	   this class is for custom rendering. The default rendering is most of the time is inadequate. This class should be derived and implemented
				based on rendering requirements and attached to the object that will be rendered with this.  

*****************************************************
*/

#ifndef SMCUSTOMRENDERER_H
#define SMCUSTOMRENDERER_H

#include "smCore/smConfig.h"
#include "smRendering/smConfigRendering.h"
#include "smCore/smCoreClass.h"
#include "smCore/smSceneObject.h"

//forward declaration
class smSceneObject;

///custom renderer for the object. 
///you could draw with one custom renderer with multiple objects.
///This is the reason why the functions are taken objects.
class smCustomRenderer:public smCoreClass{

public:
	///this function should be implemented based on objects
	//virtual void init();
	virtual void preDraw(smSceneObject* p_object);
	virtual void draw(smSceneObject *p_object);
	virtual void postDraw(smSceneObject* p_object);
};

///derive this class if you want to render a class in the viewer.
class smCustomRenderInterface:public smInterface{

	virtual void draw();
};

#endif
