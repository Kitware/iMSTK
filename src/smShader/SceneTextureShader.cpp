#include "smShader/SceneTextureShader.h"
#include "smCore/smSDK.h"


SceneTextureShader::SceneTextureShader( smChar *p_verteShaderFileName, smChar *p_fragmentFileName){
	this->log=smSDK::getErrorLog();
	this->log->isOutputtoConsoleEnabled=false;
	this->checkErrorEnabled=true;
	//smTextureManager::loadTexture
	//setShaderFileName("shaders/VertexBumpMap1.cg",NULL,"shaders/FragmentBumpMap1.cg");
	//setShaderFileName("shaders/VertexBumpMap2.cg",NULL,"shaders/FragmentBumpMap2.cg");//with shadow
	setShaderFileName(p_verteShaderFileName,NULL,p_fragmentFileName);
	createParam("depthTex");
	createParam("sceneTex");
	createParam("prevTex");

	this->checkErrorEnabled=true;
	log->isOutputtoConsoleEnabled=true;
	this->registerShader();
}


void SceneTextureShader::predraw(smMesh *p_mesh){
		
	  

}
void SceneTextureShader::handleEvent(smEvent *p_event)
{
		smKeyboardEventData *keyBoardData;
}  

void SceneTextureShader::initDraw(smDrawParam p_param){
		//addFragmentShaderParam("depthTex");
		//addFragmentShaderParam("sceneTex");
		smShader::initDraw(p_param);
	/*	timeX=getFragmentShaderParam("timeX");
		noiseTex=getFragmentShaderParam("noiseTex");*/

		this->depthTex=getFragmentShaderParam("depthTex");
		this->sceneTex=getFragmentShaderParam("sceneTex");
		this->prevTex=getFragmentShaderParam("prevTex");
				
		
	
}	
void SceneTextureShader::draw(smDrawParam p_param){

	/*	
	  this->checkShaderUpdate(2000);		
		myTime+=0.01;
		if(myTime>0.3)
		  myTime=0.0;
		smTextureManager::activateTexture("noise",2,noiseTex);
		glUniform1fARB(timeX,myTime);
		
	  */
	glPushAttrib(GL_LIGHTING_BIT|GL_ENABLE_BIT |GL_VIEWPORT_BIT);
						glDisable(GL_LIGHTING);
						glEnable(GL_DEPTH_TEST);
						glMatrixMode(GL_PROJECTION);
						glLoadIdentity();
						glOrtho(-1,1,-1,1,1,20);
				 
						glMatrixMode(GL_MODELVIEW);
						glLoadIdentity();
						glColor4f(1,1,1,1);
			 	 	 
						glTranslated(0,0,-5);
						glBegin(GL_QUADS);
						 glNormal3f(0,0,1);
						 glTexCoord2f(0,0);glVertex3f(-1,-1,0);
						 glTexCoord2f(1,0);glVertex3f(1,-1,0);
						 glTexCoord2f(1,1);glVertex3f(1,1.0,0);
						 glTexCoord2f(0,1);glVertex3f(-1,1.0,0);	   
						glEnd();	   
		glPopAttrib();


}