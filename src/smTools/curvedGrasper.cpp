#include "smTools/curvedGrasper.h"
#include "smCore/smSDK.h"


void curvedGrasper::draw(smDrawParam p_params){
	float matrix[16];
	smStylusRigidSceneObject::draw(p_params);
	
	
	smMeshContainer *containerLower = this->getMeshContainer("curvedGrasperLower");
	glPushMatrix();
		containerLower->currentMatrix.getMatrixForOpenGL(matrix);
		glMultMatrixf(matrix);
		//glTranslatef(containerLower->pos.x,this->pos.y,this->pos.z);
		//glutSolidCube(5.0);
	glPopMatrix();
}

curvedGrasper::curvedGrasper(smInt p_PhantomID, smChar *p_pivotModelFileName,smChar *p_lowerModelFileName,smChar *p_upperModelFileName){

	angle =0;

	maxangle = 10*3.14/360;
	//maxangle = 90.0*3.14/180;

	this->phantomID = p_PhantomID;


	smMatrix33<smFloat> rot;
	

	mesh_pivot = new smSurfaceMesh(SMMESH_RIGID,NULL);
	mesh_pivot->loadMesh(p_pivotModelFileName,SM_FILETYPE_3DS);
	mesh_pivot->scale(smVec3<smFloat>(0.5,0.5,0.5));
	rot.rotAroundY(-SM_PI_HALF);
	mesh_pivot->rotate(rot);
	rot.rotAroundZ(-SM_PI_HALF);
	mesh_pivot->rotate(rot);

	mesh_upperJaw = new smSurfaceMesh(SMMESH_RIGID,NULL);
	mesh_upperJaw->loadMesh(p_upperModelFileName,SM_FILETYPE_3DS);
	mesh_upperJaw->scale(smVec3<smFloat>(0.5,0.5,0.5));
	rot.rotAroundY(-SM_PI_HALF);
	mesh_upperJaw->rotate(rot);
	rot.rotAroundZ(-SM_PI_HALF);
	mesh_upperJaw->rotate(rot);

	mesh_lowerJaw = new smSurfaceMesh(SMMESH_RIGID,NULL);
	mesh_lowerJaw->loadMesh(p_lowerModelFileName,SM_FILETYPE_3DS);
	mesh_lowerJaw->scale(smVec3<smFloat>(0.5,0.5,0.5));
	rot.rotAroundY(-SM_PI_HALF);
	mesh_lowerJaw->rotate(rot);
	rot.rotAroundZ(-SM_PI_HALF);
	mesh_lowerJaw->rotate(rot);

	meshContainer_pivot.name = "curvedGrasperPivot";
	meshContainer_pivot.mesh = mesh_pivot;
		
	meshContainer_upperJaw.name = "curvedGrasperUpper";
	meshContainer_upperJaw.mesh = mesh_upperJaw;
	
	meshContainer_lowerJaw.name = "curvedGrasperLower";
	meshContainer_lowerJaw.mesh = mesh_lowerJaw;

	//for interface 
	//ADUpipeReg.regType = SOFMIS_PIPE_BYREF;
	//ADUpipeReg.listenerObject = this->mesh_lowerJaw;
	#ifdef smNIUSB6008DAQ
	NIUSB6008pipeReg.regType = SOFMIS_PIPE_BYREF;
	NIUSB6008pipeReg.listenerObject = this->mesh_lowerJaw;
	#endif

	/*
	switch(this->phantomID){
		case 0:
			this->minValue = -1.2;
			this->maxValue = 1.4;
			this->invRange = 1.0/(this->maxValue - this->minValue);
			break;
		case 1:
			this->minValue = -1.35;
			this->maxValue = 0.50;
			this->invRange = 1.0/(this->maxValue - this->minValue);
			break;
		default:
			break;
	}
	*/

	
	DAQdataID = 0;
	
}

void curvedGrasper::handleEvent(smEvent *p_event){

	smHapticOutEventData *hapticEventData;
	smKeyboardEventData *keyBoardData;
	smMatrix44d godPosMat;
	//smVec3d godPos;
	smMeshContainer *containerLower = this->getMeshContainer("curvedGrasperLower");
	smMeshContainer *containerUpper = this->getMeshContainer("curvedGrasperUpper");

	
	switch(p_event->eventType.eventTypeCode){
		case SOFMIS_EVENTTYPE_HAPTICOUT:
			
				hapticEventData=(smHapticOutEventData *)p_event->data;
				if(hapticEventData->deviceId==this->phantomID){
					
					//transRot=hapticEventData->transform;

				   smVec3d pos1=hapticEventData->transform.getColumn(3);
					//cout<<"TTTTT Object ID"<<pos.x<<","<<pos.y<<","<<pos.z<<endl;
					
				   
				    godPosMat=hapticEventData->transform;
					//godPosMat.setColumn(godPos,3);	//uncomment this line if god object is used
				   	
					/*
					godPosMat.e[0][0] = godMat[0];
					godPosMat.e[1][0] = godMat[1];
					godPosMat.e[2][0] = godMat[2];
					godPosMat.e[0][1] = godMat[3];
					godPosMat.e[1][1] = godMat[4];
					godPosMat.e[2][1] = godMat[5];
					godPosMat.e[0][2] = godMat[6];
					godPosMat.e[1][2] = godMat[7];
					godPosMat.e[2][2] = godMat[8];
					*/
					

					transRot=godPosMat;
					
					/*	if(pos1.y<0){
						//this->enableDeviceManipulatedTool=true;
						godPosMat=hapticEventData->transform;
						smVec3d  godPos=godPosMat.getColumn(3);
						godPos.y=0;
						godPosMat.setColumn(godPos,3);
						//pos.setValue(0,30,0);
						transRot=godPosMat;
					

					}
					else{
						//this->enableDeviceManipulatedTool=false;
						transRot=hapticEventData->transform;
					}
					*/
		
					pos=hapticEventData->position;

					vel=hapticEventData->velocity;

					buttonState[0] = hapticEventData->buttonState[0];
					buttonState[1] = hapticEventData->buttonState[1];

					updateOpenClose();
					
					//buttons
					containerLower->offsetRotY =  angle/360.0;//angle*maxangle;
					containerUpper->offsetRotY =  -angle/360.0;//-angle*maxangle;						

					////for interface 
					//containerLower->offsetRotY = angle * maxangle;// -(1.0-angle)*maxangle;
					//containerUpper->offsetRotY = -angle * maxangle;// (1.0-angle)*maxangle;			
				}
			
			break;
		
	



	case SOFMIS_EVENTTYPE_KEYBOARD:
					keyBoardData=(smKeyboardEventData*)p_event->data;
					
					

					if(keyBoardData->keyBoardKey==Qt::Key_1){
						
							smSDK::getInstance()->getEventDispatcher()->disableEventHandler(this,SOFMIS_EVENTTYPE_HAPTICOUT);
							this->renderDetail.renderType=this->renderDetail.renderType|SOFMIS_RENDER_NONE;
							
					}
					if(keyBoardData->keyBoardKey==Qt::Key_2){
						
						   smSDK::getInstance()->getEventDispatcher()->enableEventHandler(this,SOFMIS_EVENTTYPE_HAPTICOUT);
							this->renderDetail.renderType=this->renderDetail.renderType&(~SOFMIS_RENDER_NONE);
							
					}
				break;
		
	
	}


}

void curvedGrasper::updateOpenClose(){

	if(buttonState[0]){
		angle -= 0.05;
		if(angle<0.0) angle = 0.0;
	}

	if(buttonState[1]){
		angle += 0.05;
		if(angle>30.0) angle = 30.0;
	}


	//interface
	/*
	smMath mathUtil;

	ADUDeviceData *ADUData;

	int numElements;

	if(ADUpipeReg.data.nbrElements >0){

			numElements = ADUpipeReg.data.nbrElements;
			ADUData=(ADUDeviceData  *)ADUpipeReg.data.dataLocation;

			if(ADUData[0].deviceOpen ){
			
				///Compute open/close computations here

				angle = mathUtil.interpolate(ADUData[0].anValue[1],ADUData[0].calibration[2],ADUData[0].calibration[3]);

				//cout << angle <<endl;
				//cout<<ADUData[0].calibration[0]<<" " <<ADUData[0].calibration[1]<<endl;

				//cout<<ADUData[0].anValue[0]<<" "<<ADUData[0].anValue[1]<<endl;
			}
	}
	*/

	smFloat tF;
	#ifdef smNIUSB6008DAQ
	NIUSB6008Data *NI_Data;
	if(NIUSB6008pipeReg.data.nbrElements >0){
		NI_Data=(NIUSB6008Data  *)NIUSB6008pipeReg.data.dataLocation;
		if(NI_Data->on){

			angle = NI_Data->value[DAQdataID] * 30.0;		
			
		}
	}
	#endif
}
