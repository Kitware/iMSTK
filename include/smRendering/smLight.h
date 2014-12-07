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
    \brief	    This file has basic light structures and definitions

*****************************************************
*/

#ifndef  SMLIGHT_H
#define  SMLIGHT_H
#include "smCore/smConfig.h"
#include "smCore/smCoreClass.h"
#include "smRendering/smConfigRendering.h"

#define SMLIGHT_SPOTMAX 128

class smLight;
/// \brief light type; infinite or spotlight
enum smLightType{
	SOFMIS_LIGHT_SPOTLIGHT,
	SOFMIS_LIGHT_INFINITELIGHT
};
/// \brief location of the light type
enum smLightLocationType{
	SOFMIS_LIGHTPOS_EYE,
	SOFMIS_LIGHTPOS_WORLD
};
/// \brief light position. light at inifinite position can be defined with w
struct smLightPos{
public:
	smVec3<smFloat> pos;
	smLightPos(smFloat p_x=0.0,smFloat p_y=0.0,smFloat p_z=0.0,smFloat p_w=1.0){
		pos.x=p_x;
		pos.y=p_y;
		pos.z=p_z;
	}

	smFloat w;
	friend smLight;
};

/// \brief Basic light funtionality for the viewer..Be aware that if the light is 
///infinite the cutoff angle is still in active. You could see sudden shade in the objects
///if you don't pay attention to the cut-off angle..If you want the scene to be fully lit
///set cut-off angle 180 degrees.
///Also when the params SOFMIS_LIGHT_INFINITELIGHT,SOFMIS_LIGHTPOS_EYE are used, this means
//the light will be positioned with respect to eye coord and the light will be inifinite
//Therefore, the light should be positioned with a slight offset respect to eye pos(0,0,0);
///such as (0,0.5,0)(like a head lamp) 
struct smLight{
protected:
	smBool enabled;
	smBool previousState;
public:
	smBool drawEnabled;

	smInt renderUsage;
	QString name;
	smLightLocationType lightLocationType;
	smLightType lightType;

	smFloat attn_constant;
	smFloat attn_linear;
	smFloat attn_quadratic;

	smLight(QString p_name="", smLightType p_lightType=SOFMIS_LIGHT_INFINITELIGHT,
	        smLightLocationType p_lightLocation=SOFMIS_LIGHTPOS_EYE){
		name=p_name;
		enabled=false;
		previousState=false;
		lightPos.pos.setValue(0,0,0);
		if(p_lightType==SOFMIS_LIGHT_INFINITELIGHT)
			lightPos.w=0.0;
		else
			lightPos.w=1.0;
		lightType=p_lightType;
		lightLocationType=p_lightLocation;
		
		direction=defaultDir;
		upVector=defaultUpDir;
		transverseDir=defaultTransDir;

		direction.setValue(0,0,-1.0);
		spotCutOffAngle=45.0;
		spotExp=0.0;
		lightColorAmbient.setValue(0.2,0.2,0.2,1.0);
		lightColorDiffuse.setValue(0.8f, 0.8f, 0.8, 1.0f);
		lightColorSpecular.setValue(0.5f, 0.5f, 0.5f, 1.0f );
		drawEnabled=true;
		castShadow=false;
		shadowNearView=0.01;
		shadowFarView=4000;
		shadowRatio=1.0;
		shadorAngle=60;
		focusPosition.setValue(0,0,0);
		attn_constant=1.0;
		attn_linear=0.0;
		attn_quadratic=0.0;
	}
	/// \brief set light type
	inline void setType(smLightType p_lightType){
		if(p_lightType==SOFMIS_LIGHT_INFINITELIGHT)
			lightPos.w=0.0;
		else
			lightPos.w=1.0;
	}
	/// \brief  returns if the light is enabled or not
	inline smBool isEnabled(){
		return enabled;
	}
	/// \brief  activate the light
	inline void activate(smBool p_state){
		enabled=p_state;
		previousState=enabled;
	}
	/// \brief  light properties
	smColor lightColorDiffuse;
	smColor lightColorAmbient;
	smColor lightColorSpecular;
	
	smLightPos  lightPos;
	//smVec3<smFloat> direction;
	//between 0-1.0
	/// \brief  higher spot exponents result in a more focused light source, 
	//regardless of the spot cutoff angle. default is zeron
	smFloat spotExp;
	///angle between 0-90 and 180 is also accepted
	smFloat spotCutOffAngle;
	/// \brief light direction, up vector, transverse direction, focus point
	smVec3<smFloat> direction;
	smVec3<smFloat> upVector;
	smVec3<smFloat> transverseDir;
	smVec3<smFloat> focusPosition;//it is for shadow
	/// \brief update light direction
	void updateDirection();
	/// \brief  default direction for light, upvector and transverse direction
	static smVec3<smFloat> defaultDir;
	static smVec3<smFloat> defaultUpDir;
	static smVec3<smFloat> defaultTransDir;

	/// \brief if the light casts shadow, this should be enabled. Unfortunately, we only support one light at a time for shadows
	smBool castShadow;
	/// \brief shadow near, far,  aspect ratio and angle
	smFloat shadowNearView;
	smFloat shadowFarView;
	smFloat shadowRatio;
	smFloat shadorAngle;
};

#endif
