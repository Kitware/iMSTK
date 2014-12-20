/*
****************************************************
                  SimMedTK LICENSE
****************************************************

*****************************************************
*/

#include "smExternalDevices/smPhantomInterface.h"
#include "smRendering/smGLRenderer.h"
#include "smUtilities/smTimer.h"
#include <iostream>

HDCallbackCode HDCALLBACK hapticCallback(void *pData);
smTimer hapticTimer;

/// \brief additions to provide compabilibity with all the modules structures
void smPhantomInterface::init(){

}

/// \brief additions to provide compabilibity with all the modules structures
void smPhantomInterface::exec(){

	startDevice();
}

/// \brief
smPhantomInterface::smPhantomInterface(){

	phantomDeviceNames[0]= "PHANToM 1";
	phantomDeviceNames[1]= "PHANToM 2";
	phantomDeviceNames[2]= "PHANToM 3";
	phantomDeviceNames[3]= "PHANToM 4";

	forceEnabled=true;
	smBool openDevice = true;
	int incr =0;
	numPhantomDevices=0;
	HDErrorInfo error;

	while(openDevice){
		dHandle[incr] = hdInitDevice(phantomDeviceNames[incr].toAscii().data());
		if(HD_DEVICE_ERROR(error = hdGetError())){
			cout<<"Device "<<phantomDeviceNames[incr] <<" couldn't be opened for force feedback"<<endl;
			break;
		}
		else{
			numPhantomDevices++;
			hapticDeviceData[incr].deviceID=incr;

			cout<<"Device "<<phantomDeviceNames[incr] <<" is successfully opened for force feedback"<<endl;

			hdEnable(HD_FORCE_OUTPUT);
			hdEnable(HD_FORCE_RAMPING);

			//Stream Event Creation
			hapticEvent[incr]=new smEvent();
			hapticEventData[incr]=new smHapticOutEventData();
			hapticEvent[incr]->data=hapticEventData[incr];
			incr++;
		}
	}
}

/// \brief
smPhantomInterface::~smPhantomInterface(){

	hdStopScheduler();
	hdUnschedule(hapticCallbackHandle);
	hdWaitForCompletion(hapticCallbackHandle,HD_WAIT_INFINITE);
}

/// \brief
smInt smPhantomInterface::startDevice(){

	if(numPhantomDevices!=0){
		void *UserData = static_cast<void *>(this);
		hapticCallbackHandle = hdScheduleAsynchronous(hapticCallback,
                               UserData, HD_DEFAULT_SCHEDULER_PRIORITY);
		hapticTimer.start();
		hdStartScheduler();
		return SIMMEDTK_MSG_SUCCESS;

	}
	return SIMMEDTK_MSG_FAILURE;
}

/// \brief
int smPhantomInterface::openDevice(){

return SIMMEDTK_MSG_UNKNOWN;
}

/// \brief
int smPhantomInterface::closeDevice(){

return SIMMEDTK_MSG_UNKNOWN;
}

/// \brief
int smPhantomInterface::openDevice(smInt phantomNumber){

return SIMMEDTK_MSG_UNKNOWN;
}

/// \brief
int smPhantomInterface::getPosition(smVec3 <smDouble> &d_pos){

return SIMMEDTK_MSG_UNKNOWN;
}

/// \brief
int smPhantomInterface::getOreintation(smMatrix33 <smDouble> *d_rot){

return SIMMEDTK_MSG_UNKNOWN;
}

/// \brief
int smPhantomInterface::getDeviceTransform(smMatrix44 <smDouble> *d_transform){

return SIMMEDTK_MSG_UNKNOWN;
}


int frameCounter=0;
double totalTime=0.0;
int FPS;
smLongDouble timerPerFrame;

/// \brief
HDCallbackCode HDCALLBACK hapticCallback(void *pData){

	frameCounter++;
	smPhantomInterface  * phantomInterface = static_cast<smPhantomInterface *> (pData);
	smInt buttonState;

	smInt count =0;

	hduVector3D<double> ff;

	while (count < phantomInterface->numPhantomDevices){

		hdBeginFrame(phantomInterface->dHandle[count]);


		hdSetDoublev(HD_CURRENT_FORCE, phantomInterface->force[count]);
		hdGetDoublev(HD_CURRENT_POSITION, phantomInterface->position[count]);
		hdGetDoublev(HD_CURRENT_VELOCITY, phantomInterface->velocity[count]);
		hdGetDoublev(HD_CURRENT_JOINT_ANGLES, phantomInterface->angles[count]);
		hdGetDoublev(HD_CURRENT_TRANSFORM,phantomInterface->transform[count]);

		phantomInterface->hapticDeviceData[count].buttonState[0]=false;
		phantomInterface->hapticDeviceData[count].buttonState[1]=false;
		phantomInterface->hapticDeviceData[count].buttonState[2]=false;
		phantomInterface->hapticDeviceData[count].buttonState[3]=false;
		hdGetIntegerv(HD_CURRENT_BUTTONS,&buttonState);
		if(buttonState & HD_DEVICE_BUTTON_1){
			phantomInterface->hapticDeviceData[count].buttonState[0]=true;
		}

		if(buttonState&HD_DEVICE_BUTTON_2){
			phantomInterface->hapticDeviceData[count].buttonState[1]=true;

		}

		if (count ==0){

#ifdef USING_INTERFACE

			phantomInterface->position[count][0]+=95; //133 //66
			phantomInterface->transform[count][12]+=95;
			phantomInterface->position[count][2]+=50; //133 //66
			phantomInterface->transform[count][14]+=50;
#endif
		}

		if (count ==1){

#ifdef USING_INTERFACE

			phantomInterface->position[count][0]-=95;
			phantomInterface->transform[count][12]-=95;
#endif
		}

		/// Set the values to the data structure
		phantomInterface->hapticDeviceData[count].deviceID = count;
		phantomInterface->hapticDeviceData[count].deviceName = phantomInterface->phantomDeviceNames[count];
		phantomInterface->hapticDeviceData[count].angles.setValue(phantomInterface->angles[count][0], phantomInterface->angles[count][1],phantomInterface->angles[count][2]);
		phantomInterface->hapticDeviceData[count].position.setValue(phantomInterface->position[count][0], phantomInterface->position[count][1], phantomInterface->position[count][2]);
		phantomInterface->hapticDeviceData[count].velocity.setValue(phantomInterface->velocity[count][0], phantomInterface->velocity[count][1], phantomInterface->velocity[count][2]);
		phantomInterface->hapticDeviceData[count].transform.setMatrixFromOpenGL(phantomInterface->transform[count]);

		hdEndFrame(phantomInterface->dHandle[count]);
		count ++;
		}

	count=0;
	while (count < phantomInterface->numPhantomDevices){
		
		//phantomInterface->hapticEvent[count]=new smEvent();
		//hapticEventData[count]=new smHapticOutEventData();
		memcpy(phantomInterface->hapticEventData[count],(const void*)&phantomInterface->hapticDeviceData[count],sizeof(phantomInterface->hapticDeviceData[count]));
		phantomInterface->hapticEvent[count]->eventType=SIMMEDTK_EVENTTYPE_HAPTICOUT;
		phantomInterface->hapticEvent[count]->senderId=phantomInterface->getModuleId();
		phantomInterface->hapticEvent[count]->senderType=SIMMEDTK_SENDERTYPE_MODULE;
		//phantomInterface->hapticEvent[count]->data=phantomInterface->hapticEventData[count];
		count++;
	}

	count=0;
	while (count < phantomInterface->numPhantomDevices){
		if(phantomInterface->eventDispatcher)
		phantomInterface->eventDispatcher->sendStreamEvent(phantomInterface->hapticEvent[count]);
		count++;		
	}

	if(phantomInterface->terminateExecution){
		phantomInterface->terminationCompleted=true;			
		return HD_CALLBACK_DONE;
	}

	timerPerFrame=hapticTimer.now(SIMMEDTK_TIMER_INMILLISECONDS);
	frameCounter++;
	if(SMTIMER_FRAME_MILLISEC2SECONDS(timerPerFrame)>=1.0){
		hapticTimer.start();
		frameCounter=0;
		totalTime=0.0;
	}

	return HD_CALLBACK_CONTINUE;
}

/// \brief
void smPhantomInterface::handleEvent(smEvent *p_event){

	smHapticInEventData *hapticEventData;
	switch(p_event->eventType.eventTypeCode){
		case  SIMMEDTK_EVENTTYPE_HAPTICIN:
			if(forceEnabled){
				hapticEventData=(smHapticInEventData *)p_event->data;
				force[hapticEventData->deviceId][0]=	hapticEventData->force.x;
				force[hapticEventData->deviceId][1]=	hapticEventData->force.y;
				force[hapticEventData->deviceId][2]=	hapticEventData->force.z;
			}
			break;
	}
}

/// \brief
void smPhantomInterface::draw(smDrawParam p_params){

	static smVec3<smFloat> xAxis;
	static smVec3<smFloat> yAxis;
	static smVec3<smFloat> zAxis;
	static smVec3<smFloat> pos;

	for(smInt i=0;i<numPhantomDevices;i++){
		pos=hapticDeviceData[i].position;
		xAxis=hapticDeviceData[i].transform.getColumn(0);
		yAxis=hapticDeviceData[i].transform.getColumn(1);
		zAxis=hapticDeviceData[i].transform.getColumn(2);
		smGLRenderer::drawCoordSystem(p_params.rendererObject,hapticDeviceData[i].deviceName,pos,xAxis,yAxis,zAxis);
	}
}