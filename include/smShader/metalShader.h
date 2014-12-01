#ifndef METALSHADER_H
#define METALSHADER_H

#include "smShader/smShader.h"

class MetalShader:public smShader, public smEventHandler{
public:
	smGLInt lightPower;
	smGLInt roughness;
	smGLInt tangent;
	smGLInt specularPower;
	smGLFloat specularPowerValue;
	smInt attrib;
	smGLInt alphaMapGain;
	smGLFloat alphaMapGainValue;
	smGLInt canGetShadowUniform;
	
	MetalShader(smChar *p_verteShaderFileName="shaders/VertexBumpMap1.cg",
	            smChar *p_fragmentFileName="shaders/FragmentBumpMap1.cg"){
		this->log=smSDK::getErrorLog();
		this->log->isOutputtoConsoleEnabled=false;
		this->checkErrorEnabled=true;
		setShaderFileName(p_verteShaderFileName,NULL,p_fragmentFileName);
		createParam("DecalTex");
		createParam("BumpTex");
		createParam("SpecularTex");
		createParam("DispTex");
		createParam("OCCTex");
		createParam("noiseTex");
		createParam("specularPower");
		createParam("alphaMap");
		createParam("alphaMapGain");
		createParam("canGetShadow");
		createAttrib("tangent");

		//if the objtets are static we cannot change this value during runtime
		specularPowerValue=5.0;
		alphaMapGain=-1;
		alphaMapGainValue=1.0;
		this->checkErrorEnabled=true;
		log->isOutputtoConsoleEnabled=true;
	}

	void attachMesh(smMesh* p_mesh, smChar *p_bump,
	                smChar *p_decal, smChar *p_specular,
	                smChar *p_OCC, smChar *p_disp){
		if(!attachTexture(p_mesh->uniqueId,p_bump,"BumpTex"))
			cout<<"Error in bump attachment for mesh:"<<p_mesh->name.toStdString().c_str()<<endl;

		attachTexture(p_mesh->uniqueId,p_decal,"DecalTex");
		attachTexture(p_mesh->uniqueId,p_specular,"SpecularTex");
		attachTexture(p_mesh->uniqueId,p_OCC,"OCCTex");
		attachTexture(p_mesh->uniqueId,p_disp,"DispTex");
	}

	void attachMesh(smMesh* p_mesh,smChar *p_bump,smChar *p_decal,smChar *p_specular,smChar *p_OCC,smChar *p_disp,smChar *p_alphaMap){
		attachTexture(p_mesh->uniqueId,p_bump,"BumpTex");
		attachTexture(p_mesh->uniqueId,p_decal,"DecalTex");
		attachTexture(p_mesh->uniqueId,p_specular,"SpecularTex");
		attachTexture(p_mesh->uniqueId,p_OCC,"OCCTex");
		attachTexture(p_mesh->uniqueId,p_disp,"DispTex");
		attachTexture(p_mesh->uniqueId,p_alphaMap,"AlphaTex");
	}

	void draw(smDrawParam p_param){
		//placeholder
	}

	virtual void initDraw(smDrawParam p_param){
		smShader::initDraw(p_param);
		specularPower=this->getFragmentShaderParam("specularPower");
		alphaMapGain=this->getFragmentShaderParam("alphaMapGain");
		this->tangentAttrib=this->getShaderAtrribParam("tangent");
		canGetShadowUniform=getFragmentShaderParam("canGetShadow");
	}

	virtual void predraw(smMesh *mesh){
		specularPowerValue=mesh->renderDetail.shininess;
		glUniform1fARB(specularPower,specularPowerValue);
		glUniform1fARB(alphaMapGain,alphaMapGainValue);

		if(mesh->renderDetail.canGetShadow)
			glUniform1fARB(canGetShadowUniform,1);
		else
			glUniform1fARB(canGetShadowUniform,0);
	}

	void handleEvent(smEvent *p_event){
		smKeyboardEventData *keyBoardData;

		switch(p_event->eventType.eventTypeCode){
		case SOFMIS_EVENTTYPE_KEYBOARD:
			keyBoardData = (smKeyboardEventData*)p_event->data;
			if(keyBoardData->keyBoardKey==Qt::Key_Plus){
				specularPowerValue+=5;
				cout<<specularPowerValue<<endl;
			}

			if(keyBoardData->keyBoardKey==Qt::Key_Minus){
				specularPowerValue-=5;
				cout<<specularPowerValue<<endl;
			}

			break;
		}
	}

	virtual void switchEnable(){
		//
	}
	virtual void switchDisable(){
		//
	}
};

class MetalShaderShadow:public MetalShader{
	smGLInt shadowMapUniform;
	smGLInt canGetShadowUniform;

public:
	MetalShaderShadow(smChar*p_vertexShaderFileName="shaders/MultipleShadowsVertexBumpMap2.cg",
	                  smChar*p_fragmentShaderFileName="shaders/MultipleShadowsFragmentBumpMap2.cg"):
	                  MetalShader(p_vertexShaderFileName,p_fragmentShaderFileName)
	{
		createParam("ShadowMapTEST");
		createParam("canGetShadow");
	}

	virtual void initDraw(smDrawParam p_param){
		MetalShader::initDraw(p_param);
		this->print();
		shadowMapUniform=getFragmentShaderParam("ShadowMapTEST");
		canGetShadowUniform=getFragmentShaderParam("canGetShadow");
	}

	virtual void predraw(smMesh *p_mesh){
		MetalShader::predraw(p_mesh);
		if(p_mesh->renderDetail.canGetShadow)
			glUniform1fARB(canGetShadowUniform,1);
		else
			glUniform1fARB(canGetShadowUniform,0);
		smTextureManager::activateTexture("depth",30,shadowMapUniform);
	}
};

class MetalShaderSoftShadow:public MetalShader{

	smGLInt shadowMapUniform;
public:
	MetalShaderSoftShadow():
	    MetalShader("shaders/SingleShadowVertexBumpMap2.cg",
	                "shaders/SingleShadowFragmentBumpMap2.cg"){
		createParam("ShadowMapTEST");
	}

	virtual void initDraw(smDrawParam p_param){
		MetalShader::initDraw(p_param);
		this->print();
		shadowMapUniform=getFragmentShaderParam("ShadowMapTEST");
	}

	virtual void predraw(smMesh *p_mesh){
		MetalShader::predraw(p_mesh);
		smTextureManager::activateTexture("depth",30,shadowMapUniform);
	}
};

#endif
