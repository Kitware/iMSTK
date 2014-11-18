#ifndef SCENETEXTURESHADER_H
#define SCENETEXTURESHADER_H

#include "smShader/smShader.h"
#include "smCore/smEvent.h"
#include "smCore/smEventHandler.h"
#include "smCore/smTextureManager.h"



class SceneTextureShader:public smShader, public smEventHandler{
public:
	smGLInt depthTex;
	smGLInt sceneTex;
	smGLInt  prevTex;
	//smErrorLog *log;

	/*
	 smGLInt timeX;
     smGLInt noiseTex;
     smFloat myTime;
	  */
	SceneTextureShader( smChar *p_verteShaderFileName="shaders/renderSceneVertexShader.glsl",
	                    smChar *p_fragmentFileName="shaders/renderSceneFragShader.glsl");

	void initDraw(smDrawParam p_param);	  
	void draw(smDrawParam p_param);
	virtual void predraw(smMesh *p_mesh);
	virtual void handleEvent(smEvent *p_event);
	/*{
		smKeyboardEventData *keyBoardData;
    }  */

};



#endif