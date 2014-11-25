#include <assert.h>
#include "smRenderEffects/smEffectSim.h"
#include "smRendering/smViewer.h"
#include "smCore/smEventHandler.h"
#include "smCore/smEventData.h"

float fadeEffect =0.0; 

/// \brief
void smEffectSim::run(){

}

/// \brief
void transform2ScreenCoord(double x,double y,double z,GLdouble &winX,GLdouble &winY,GLdouble &winZ){

	GLdouble modelview[16];
	GLdouble projection[16];
	GLint viewport[4];	

	glGetDoublev(GL_MODELVIEW_MATRIX , modelview);
	glGetDoublev(GL_PROJECTION_MATRIX, projection);
	glGetIntegerv( GL_VIEWPORT, viewport );
	gluProject(x,y,z,modelview,projection,viewport,&winX,&winY,&winZ);

	winX=winX/viewport[2];
	winY=winY/viewport[3];
}

/// \brief
void transFormPoint(double  *modelViewMatrix,  smVec3f src,smVec3f &dst){

	float dstVec[4];
	float srcVec[4];

	srcVec[0]=src[0];
	srcVec[1]=src[1];
	srcVec[2]=src[2];
	srcVec[3]=1;

	float resMat[4][4];
	for(int j=0;j<4;j++)
		for(int i=0;i<4;i++){
			resMat[i][j]=modelViewMatrix[j*4+i];
		} 

		dstVec[0]=resMat[0][0]*srcVec[0]+resMat[0][1]*srcVec[1]+resMat[0][2]*srcVec[2]+resMat[0][3]*srcVec[3];
		dstVec[1]=resMat[1][0]*srcVec[0]+resMat[1][1]*srcVec[1]+resMat[1][2]*srcVec[2]+resMat[1][3]*srcVec[3];
		dstVec[2]=resMat[2][0]*srcVec[0]+resMat[2][1]*srcVec[1]+resMat[2][2]*srcVec[2]+resMat[2][3]*srcVec[3];
		dstVec[3]=resMat[3][0]*srcVec[0]+resMat[3][1]*srcVec[1]+resMat[3][2]*srcVec[2]+resMat[3][3]*srcVec[3];

		dst.x=dstVec[0]/dstVec[3];
		dst.y=dstVec[1]/dstVec[3];
		dst.z=dstVec[2]/dstVec[3];
}

SMOKESTATE smokeState=SMOKE_NOTSTARTARTED;

/// \brief
void smEffectSim::draw(smDrawParam p_params){

	glPushAttrib(GL_ALL_ATTRIB_BITS);
		glEnable (GL_BLEND);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		double myLeft[16];
		double normalizedToolTipX,normalizedToolTipY,depth;
		double offset=-0.0;
		double toolX,toolY,toolZ;
		smVec3f dst;

		//model->phantomInterface->getRightHandMatrix(left);
		toolX=left[12]+left[8]*offset;
		toolY=left[13]+left[9]*offset;
		toolZ=left[14]+left[10]*offset;


		transform2ScreenCoord(toolX,toolY,toolZ,normalizedToolTipX,normalizedToolTipY,depth);
		p_params.rendererObject->camera()->getModelViewMatrix(myLeft);
		transFormPoint(myLeft,smVec3f(toolX,toolY,toolZ),dst);
		depth=dst.z/p_params.rendererObject->camera()->zFar();

		if(buttonState){
			switch (smokeState){
				case SMOKE_NOTSTARTARTED:
				case SMOKE_FADE:

					smoke->drawSmokeVideo(true,true,fadeEffect,normalizedToolTipX,normalizedToolTipY,depth,toolX,toolY,toolZ,p_params);
					smokeState=SMOKE_CONTINUES;

					break;
				case SMOKE_CONTINUES:
					fadeEffect=(fadeEffect<1.0?fadeEffect+0.01:1.0);
					smoke->drawSmokeVideo(true,true,fadeEffect,normalizedToolTipX,normalizedToolTipY,depth,toolX,toolY,toolZ,p_params);
					break;
			}
		}
		else{
			switch (smokeState){
				case SMOKE_CONTINUES:
					smoke->drawSmokeVideo(true,true,fadeEffect,normalizedToolTipX,normalizedToolTipY,depth,toolX,toolY,toolZ,p_params);
					smokeState=SMOKE_FADE;
					break;
				case SMOKE_FADE:
					fadeEffect=(fadeEffect>0?fadeEffect-0.01:0);
					smoke->drawSmokeVideo(true,true,fadeEffect,normalizedToolTipX,normalizedToolTipY,depth,toolX,toolY,toolZ,p_params);
					break;
			}
		}
	glPopAttrib();
}

/// \brief
void smEffectSim::handleEvent(smEvent *p_event){

	smMatrix44d godPosMat;
	smVec3d godPos;
	smHapticOutEventData *hapticEventData;
	smKeyboardEventData *keyBoardData;

	switch(p_event->eventType.eventTypeCode){
		case SOFMIS_EVENTTYPE_HAPTICOUT:
			
			hapticEventData=(smHapticOutEventData *)p_event->data;
			if(hapticEventData->deviceId==0){
				hapticEventData->transform.getMatrixForOpenGL(left);
			}
			break;

		case SOFMIS_EVENTTYPE_KEYBOARD:

			keyBoardData = (smKeyboardEventData*)p_event->data;
			if(keyBoardData->keyBoardKey==Qt::Key_8){
				buttonState = true;
			}

			if(keyBoardData->keyBoardKey==Qt::Key_9){
				buttonState = false;
			}
			break;
	}
}
