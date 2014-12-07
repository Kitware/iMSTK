#ifndef SCENETEXTURESHADER_H
#define SCENETEXTURESHADER_H

#include "smShader/smShader.h"
#include "smCore/smEvent.h"
#include "smCore/smEventHandler.h"
#include "smCore/smTextureManager.h"
/// \brief scene texture shader. This shader works on the scene that is placed on 2D image. It is for image based effects
class SceneTextureShader:public smShader, public smEventHandler{
public:
	/// \brief depth  texture GL id that is sent to shader. It stores scene depth values  
	smGLInt depthTex;
	/// \brief scene  texture GL id that is sent to shader. It stores scene RGB values  
	smGLInt sceneTex;
	/// \brief previous  texture GL id that is sent to shader. It stores previous image RGB values rendered in previous rendering stage in multipass  
	smGLInt  prevTex;
	/// \brief constructor that receives the vertex and fragment shader file names 
	SceneTextureShader(smChar *p_verteShaderFileName="shaders/renderSceneVertexShader.glsl",
	                   smChar *p_fragmentFileName="shaders/renderSceneFragShader.glsl");
	/// \brief called during  rendering initialization
	void initDraw(smDrawParam p_param);
	/// \brief if object is added to viewer, it is called from viewer. used for debuginng purposes. It is needed to see effect of instant code changes in shader code 
	void draw(smDrawParam p_param);

	/// \brief pre drawing of the shader. used for binding the uniforms if there are.
	virtual void predraw(smMesh *p_mesh);
	/// \brief handle the events 
	virtual void handleEvent(smEvent *p_event);
};

#endif
