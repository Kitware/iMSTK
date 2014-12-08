#ifndef SMMOTIONTRANSFORMER_H
#define SMMOTIONTRANSFORMER_H

#include "smCore/smSDK.h"
#include "smCore/smEvent.h"
#include "smCore/smEventHandler.h"
#include "smUtilities/smQuat.h"

enum smMotionTransType{
	SM_TRANSMOTION_HAPTIC2CAM,
};

class smMotionTransformer: public smCoreClass,public smEventHandler{
public:
	smBool enabled;
};

class smHapticTrans:public smMotionTransformer{
protected:
	smEvent *newEvent;
	smFloat motionScale;
	smInt deviceId;
	smVec3<smDouble> defaultDirection;
	smVec3<smDouble> defaultUpDirection;
	smVec3<smDouble> transFormedDirection;
	smVec3<smDouble> transFormedUpDirection;
	smEventDispatcher *dispatch;
public:
	smHapticTrans(){
		newEvent=new smEvent();
		defaultDirection.setValue(0,0,-1);
		defaultUpDirection.setValue(0,1.0,0);
		dispatch=(smSDK::getInstance())->getEventDispatcher();
		motionScale=1.0;
		enabled=true;
	}

	void setDeviceIdToListen(smInt p_id){
		deviceId=p_id;
	}

	void setMotionScale(smFloat p_scale){
		motionScale=p_scale;
	}

	inline void computeTransformation(smMatrix44<smDouble> &p_mat){
		static smMatrix33<smDouble> mat;
		mat=p_mat;
		transFormedDirection=(mat*defaultDirection);
		transFormedUpDirection=(mat*defaultUpDirection);
	}

	inline void sendEvent(){
		dispatch->sendStreamEvent(newEvent);
	}
};

class smHapticCameraTrans:public smHapticTrans {
protected:
	smQuatd quat;
public:
	smDouble offsetAngle_RightDirection;
	smDouble offsetAngle_UpDirection;

	smHapticCameraTrans(smInt p_deviceID=0){
		deviceId=p_deviceID;
		newEvent->data=new smCameraEventData();
		newEvent->eventType=SOFMIS_EVENTTYPE_CAMERA_UPDATE;
		dispatch->registerEventHandler(this,SOFMIS_EVENTTYPE_HAPTICOUT);
		offsetAngle_RightDirection=0;
		offsetAngle_UpDirection=0;
	}

	inline void handleEvent(smEvent *p_event){
		smHapticOutEventData *hapticEventData;
		smVec3d rightVector;
		switch(p_event->eventType.eventTypeCode){
		case SOFMIS_EVENTTYPE_HAPTICOUT:
			if(!enabled)
				return;

			hapticEventData=(smHapticOutEventData *)p_event->data;
			if(hapticEventData->deviceId==deviceId){
				((smCameraEventData*)newEvent->data)->pos=hapticEventData->position*motionScale;
				computeTransformation(hapticEventData->transform);
				
				((smCameraEventData*)newEvent->data)->direction=transFormedDirection;
				((smCameraEventData*)newEvent->data)->upDirection=transFormedUpDirection;
				rightVector=transFormedDirection.cross(transFormedUpDirection);
				rightVector.normalize();

				quat.fromAxisAngle(rightVector,SM_DEGREES2RADIANS(offsetAngle_RightDirection));
				
				((smCameraEventData*)newEvent->data)->direction=quat.rotate(((smCameraEventData*)newEvent->data)->direction);
				((smCameraEventData*)newEvent->data)->upDirection=quat.rotate(((smCameraEventData*)newEvent->data)->upDirection);
				sendEvent();
			}
			break;
		}
	}
};

class smHapticLightTrans:public smHapticTrans {
protected:
	smInt lightIndex;
public:
	void setLightIndex(smInt p_lightIndex){
		lightIndex=p_lightIndex;
	}

	smHapticLightTrans(smInt p_id=0){
		deviceId=p_id;
		lightIndex=0;
		newEvent->data=new smLightMotionEventData();
		newEvent->eventType=SOFMIS_EVENTTYPE_LIGHTPOS_UPDATE;
		dispatch->registerEventHandler(this,SOFMIS_EVENTTYPE_HAPTICOUT);
	}

	inline void handleEvent(smEvent *p_event){
		smHapticOutEventData *hapticEventData;
		switch(p_event->eventType.eventTypeCode){
		case SOFMIS_EVENTTYPE_HAPTICOUT:
			if(!enabled)
				return;

			hapticEventData=(smHapticOutEventData *)p_event->data;
			if(hapticEventData->deviceId==deviceId){	
				((smLightMotionEventData*)newEvent->data)->lightIndex=lightIndex;
				((smLightMotionEventData*)newEvent->data)->pos=hapticEventData->position*motionScale;
				computeTransformation(hapticEventData->transform);
				((smLightMotionEventData*)newEvent->data)->direction=transFormedDirection;
				
				sendEvent();

			}
			break;
		}
	}
};

#endif
