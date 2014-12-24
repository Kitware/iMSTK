/*****************************************************
                  SIMMEDTK LICENSE
****************************************************

*****************************************************/

#ifndef SMCUSTOMRENDERER_H
#define SMCUSTOMRENDERER_H

#include "smCore/smConfig.h"
#include "smRendering/smConfigRendering.h"
#include "smCore/smCoreClass.h"
#include "smCore/smSceneObject.h"

//forward declaration
class smSceneObject;

/// \brief custom renderer for the object. 
///you could draw with one custom renderer with multiple objects.
///This is the reason why the functions are taken objects.
class smCustomRenderer:public smCoreClass{

public:
	/// \briefthis function should be implemented based on objects
	//virtual void init();
	/// \brief draw pre, default and post draw routines
	virtual void preDraw(smSceneObject* p_object);
	virtual void draw(smSceneObject *p_object);
	virtual void postDraw(smSceneObject* p_object);
};

/// \briefderive this class if you want to render a class in the viewer.
class smCustomRenderInterface:public smInterface{

	virtual void draw();
};

#endif
