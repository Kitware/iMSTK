#include <GL/glew.h>
#include <string.h>
#include "smRendering/smFrameBuffer.h"
#include "smCore/smConfig.h"
#include "smCore/smTextureManager.h"
#include "smRendering/smGLRenderer.h"
//#include "smViewer.h"
 
  

smBool smFrameBuffer::checkStatus(){
	GLenum ret= glCheckFramebufferStatus(GL_FRAMEBUFFER);
	switch(ret)
    {
    case GL_FRAMEBUFFER_COMPLETE_EXT:
        std::cout << "Framebuffer complete." << std::endl;
        return true;

    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
        std::cout << "[ERROR] Framebuffer incomplete: Attachment is NOT complete." << std::endl;
        return false;

    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
        std::cout << "[ERROR] Framebuffer incomplete: No image is attached to FBO." << std::endl;
        return false;

    case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
        std::cout << "[ERROR] Framebuffer incomplete: Attached images have different dimensions." << std::endl;
        return false;

    case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
        std::cout << "[ERROR] Framebuffer incomplete: Color attached images have different internal formats." << std::endl;
        return false;

    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
        std::cout << "[ERROR] Framebuffer incomplete: Draw buffer." << std::endl;
        return false;

    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
        std::cout << "[ERROR] Framebuffer incomplete: Read buffer." << std::endl;
        return false;

    case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
        std::cout << "[ERROR] Unsupported by FBO implementation." << std::endl;
        return false;

    default:
        std::cout << "[ERROR] Unknow error." << std::endl;
        return false;
    }

}	


void test(){

	
			 glPushAttrib(GL_TEXTURE_BIT|GL_VIEWPORT_BIT|GL_LIGHTING_BIT);
			 //glUseProgramObjectARB(0);
			 //glEnable(GL_LIGHTING);
			 
			 glDisable(GL_LIGHTING);
			 glMatrixMode(GL_PROJECTION);
			 glLoadIdentity();
			 glOrtho(-1,1,-1,1,1,20);
			 glViewport(0,0,1680,1050);
			 //gluPerspective(45,1.0,0.001,1000);
			 glMatrixMode(GL_MODELVIEW);
			 glLoadIdentity();
			 glColor4f(1,1,1,1);
			 
			 
			 //glActiveTextureARB(GL_TEXTURE0);
			 //smTextureManager::activateTextureGL(_fbo.GetAttachedId(GL_DEPTH_ATTACHMENT_EXT),0);
			 //glEnable(GL_TEXTURE_2D);
			 
			 glTranslated(0,0,-1);
			// glRotatef(45,1,1,0);

				glBegin(GL_QUADS);
					 glNormal3f(0,0,1);
					 glTexCoord2d(0,0);glVertex3f(-1,-1,0);
					 glTexCoord2d(1,0);glVertex3f(1,-1,0);
					 glTexCoord2d(1,1);glVertex3f(1,1.0,0);
					 glTexCoord2d(0,1);glVertex3f(-1,1.0,0);	   

					
				 
				 
				 glEnd();	   
				 
			 //glDisable(GL_TEXTURE_2D);
			glPopAttrib();


}
void smFrameBuffer::draw(smDrawParam p_params){
	//smInt width=p_params.rendererObject->camera()->screenWidth();
	//smInt height=p_params.rendererObject->camera()->screenHeight();

	
	//	 test();
		 	 glPushAttrib(GL_TEXTURE_BIT|GL_VIEWPORT_BIT|GL_LIGHTING_BIT);
			 //glUseProgramObjectARB(0);
			 //glEnable(GL_LIGHTING);
			 
			 glDisable(GL_LIGHTING);
			 glMatrixMode(GL_PROJECTION);
			 glLoadIdentity();
			 glOrtho(-1,1,-1,1,1,20);
			 //glViewport(0,0,1680,1050);
			 //glViewport(0,0,this->width,this->height);
			 //gluPerspective(45,1.0,0.001,1000);
			 glMatrixMode(GL_MODELVIEW);
			 glLoadIdentity();
			 glColor4f(1,1,1,1);
			 
			 
			 //glActiveTextureARB(GL_TEXTURE0);
			 //smTextureManager::activateTextureGL(_fbo.GetAttachedId(GL_DEPTH_ATTACHMENT_EXT),0);
			 //glEnable(GL_TEXTURE_2D);
			 
			 glTranslated(0,0,-1);
			// glRotatef(45,1,1,0);

				glBegin(GL_QUADS);
					 glNormal3f(0,0,1);
					 glTexCoord2d(0,0);glVertex3f(-1,-1,0);
					 glTexCoord2d(1,0);glVertex3f(1,-1,0);
					 glTexCoord2d(1,1);glVertex3f(1,1.0,0);
					 glTexCoord2d(0,1);glVertex3f(-1,1.0,0);	   

					
				 
				 
				 glEnd();	   
				 
			 //glDisable(GL_TEXTURE_2D);
			glPopAttrib();




































			// glPushAttrib(GL_TEXTURE_BIT|GL_VIEWPORT_BIT|GL_LIGHTING_BIT);
			// //glUseProgramObjectARB(0);
			// //glEnable(GL_LIGHTING);
			// smGLRenderer::
			// 
			// glDisable(GL_LIGHTING);
			//glMatrixMode(GL_PROJECTION);
			//glPushMatrix();
			//	glLoadIdentity();
			//	glOrtho(-1,1,-1,1,1,20);
			// //gluPerspective(45,1.0,0.001,1000);
			//glMatrixMode(GL_MODELVIEW);
			//glPushMatrix();
			//	glLoadIdentity();
			//	glColor4f(1,1,1,1);
			// 
			//	 glEnable(GL_TEXTURE_2D);
			//	 glActiveTextureARB(GL_TEXTURE0);
			//	 smTextureManager::activateTextureGL(_fbo.GetAttachedId(GL_DEPTH_ATTACHMENT_EXT),0);
			//	 
			//	 
			//	 glTranslated(0,0,-10);
			//	// glRotatef(45,1,1,0);
			//	 
			//	 ////glutSolidTeapot(2);
			//	 // glBegin(GL_QUADS);
			//		//	glColor4f(1, 1, 1, 1);

			//		//	// face v0-v1-v2-v3
			//		//	glNormal3f(0,0,1);
			//		//	glTexCoord2f(1, 1);  glVertex3f(1,1,1);
			//		//	glTexCoord2f(0, 1);  glVertex3f(-1,1,1);
			//		//	glTexCoord2f(0, 0);  glVertex3f(-1,-1,1);
			//		//	glTexCoord2f(1, 0);  glVertex3f(1,-1,1);

			//		//	// face v0-v3-v4-v5
			//		//	glNormal3f(1,0,0);
			//		//	glTexCoord2f(0, 1);  glVertex3f(1,1,1);
			//		//	glTexCoord2f(0, 0);  glVertex3f(1,-1,1);
			//		//	glTexCoord2f(1, 0);  glVertex3f(1,-1,-1);
			//		//	glTexCoord2f(1, 1);  glVertex3f(1,1,-1);

			//		//	// face v0-v5-v6-v1
			//		//	glNormal3f(0,1,0);
			//		//	glTexCoord2f(1, 0);  glVertex3f(1,1,1);
			//		//	glTexCoord2f(1, 1);  glVertex3f(1,1,-1);
			//		//	glTexCoord2f(0, 1);  glVertex3f(-1,1,-1);
			//		//	glTexCoord2f(0, 0);  glVertex3f(-1,1,1);

			//		//	// face  v1-v6-v7-v2
			//		//	glNormal3f(-1,0,0);
			//		//	glTexCoord2f(1, 1);  glVertex3f(-1,1,1);
			//		//	glTexCoord2f(0, 1);  glVertex3f(-1,1,-1);
			//		//	glTexCoord2f(0, 0);  glVertex3f(-1,-1,-1);
			//		//	glTexCoord2f(1, 0);  glVertex3f(-1,-1,1);

			//		//	// face v7-v4-v3-v2
			//		//	glNormal3f(0,-1,0);
			//		//	glTexCoord2f(0, 0);  glVertex3f(-1,-1,-1);
			//		//	glTexCoord2f(1, 0);  glVertex3f(1,-1,-1);
			//		//	glTexCoord2f(1, 1);  glVertex3f(1,-1,1);
			//		//	glTexCoord2f(0, 1);  glVertex3f(-1,-1,1);

			//		//	// face v4-v7-v6-v5
			//		//	glNormal3f(0,0,-1);
			//		//	glTexCoord2f(0, 0);  glVertex3f(1,-1,-1);
			//		//	glTexCoord2f(1, 0);  glVertex3f(-1,-1,-1);
			//		//	glTexCoord2f(1, 1);  glVertex3f(-1,1,-1);
			//		//	glTexCoord2f(0, 1);  glVertex3f(1,1,-1);
			//		//glEnd();
			//		glBegin(GL_QUADS);
			//			 glNormal3f(0,0,1);
			//			 glTexCoord2d(0,0);glVertex3f(0,0,0);
			//			 glTexCoord2d(1,0);glVertex3f(1,0,0);
			//			 glTexCoord2d(1,1);glVertex3f(1,1.0,0);
			//			 glTexCoord2d(0,1);glVertex3f(0,1.0,0);	   
			//		 glEnd();	   
			//		 
			//		//glDisable(GL_TEXTURE_2D);
			//		glPopAttrib();
			//glMatrixMode(GL_MODELVIEW);
			//glPopMatrix();
			//glMatrixMode(GL_PROJECTION);
			//glPopMatrix();

		//if(isDepthTexAttached&&renderDepthBuff){
		//	  
		//	 glPushAttrib(GL_TEXTURE_BIT|GL_VIEWPORT_BIT|GL_LIGHTING_BIT);
		//	 glUseProgramObjectARB(0);
		//	 glDisable(GL_LIGHTING);
		//	 glMatrixMode(GL_PROJECTION);
		//	 glPushMatrix();
		//	 glLoadIdentity();
		//	 glOrtho(-1,1,-1,1,1,20);
		//	 glMatrixMode(GL_MODELVIEW);
		//	 glPushMatrix();
		//	 glLoadIdentity();
		//	 glColor4f(1,1,1,1);
		//	 glEnable(GL_TEXTURE_2D);
		//	 
		//	 //glActiveTextureARB(GL_TEXTURE0);
		//	 //smTextureManager::activateTextureGL(_fbo.GetAttachedId(GL_DEPTH_ATTACHMENT_EXT),0);
		//	 smTextureManager::activateTexture("depth");
		//	 //glEnable(GL_TEXTURE_2D);
		//	 glTranslated(0,0,-1);
		//	 	 glBegin(GL_QUADS);
		//			 glNormal3f(0,0,1);
		//			 glTexCoord2d(0,0);glVertex3f(0,0,0);
		//			 glTexCoord2d(1,0);glVertex3f(1,0,0);
		//			 glTexCoord2d(1,1);glVertex3f(1,1.0,0);
		//			 glTexCoord2d(0,1);glVertex3f(0,1.0,0);	   

		//			
		//		 
		//		 
		//		 glEnd();
		//	 glDisable(GL_TEXTURE_2D);

		//	 glMatrixMode(GL_MODELVIEW);
		//	 glPopMatrix();
		//	 glMatrixMode(GL_PROJECTION);
		//	 glPopMatrix();

		//	glPopAttrib();
		//}
		//if(isColorBufAttached&&renderColorBuff){
		//	  
		//	 glPushAttrib(GL_TEXTURE_BIT|GL_VIEWPORT_BIT|GL_LIGHTING_BIT);
		//	 //glUseProgramObjectARB(0);
		//	 glDisable(GL_LIGHTING);
		//	 glMatrixMode(GL_PROJECTION);
		//	 glLoadIdentity();
		//	 glOrtho(-1,1,-1,1,1,20);
		//	 glMatrixMode(GL_MODELVIEW);
		//	 glLoadIdentity();
		//	 glColor4f(1,1,1,1);
		//	 
		//	 
		//	 //glActiveTextureARB(GL_TEXTURE0);
		//	 smTextureManager::activateTextureGL(_fbo.GetAttachedId(GL_COLOR_ATTACHMENT0+defaultColorAttachment),0);
		//	 glEnable(GL_TEXTURE_2D);
		//	 glTranslated(0,0,-1);
		//		 glBegin(GL_QUADS);
		//		    /*
		//			 glTexCoord2d(0,0);glVertex3f(0,0,0);
		//			 glTexCoord2d(1,0);glVertex3f(1,0,0);
		//			 glTexCoord2d(1,1);glVertex3f(1,1.0,0);
		//			 glTexCoord2d(0,1);glVertex3f(0,1.0,0);
		//			 */
		//			 glTexCoord2d(0,0);glVertex3f(-1,-1,0);
		//			 glTexCoord2d(1,0);glVertex3f(1,-1,0);
		//			 glTexCoord2d(1,1);glVertex3f(1,1.0,0);
		//			 glTexCoord2d(0,1);glVertex3f(-1,1.0,0);  
		//		 
		//		 
		//		 glEnd();
		//	 glDisable(GL_TEXTURE_2D);
		//	glPopAttrib();
		//}
	
	
	}