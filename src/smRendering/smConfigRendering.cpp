#include "smRendering/smConfigRendering.h"
#include "smShader/smShader.h"

smColor smColor::colorWhite(1.0,1.0,1.0,1.0);
smColor smColor::colorBlue (0.0,0.0,1.0,1.0);
smColor smColor::colorGreen(0.0,1.0,0.0,1.0);
smColor smColor::colorRed  (1.0,0.0,0.0,1.0);
smColor smColor::colorGray (0.8,0.8,0.8,1.0);
smColor smColor::colorYellow(1,1,0,1);
smColor smColor::colorPink(1,0,1,1);


// smBool smShaderAttachment::operator ==(QString &p_string){
//
//	 return (shader->getName()==p_string);
//}
//
//
// inline smBool smShaderAttachment::operator ==(smShaderAttachment &p_param)
//{
//	 //return (shader-> ==p_param.shader->name);
//	return (shader->uniqueId==p_param.shader->uniqueId);
//	
//}
//
void smRenderDetail::addShader(smUnifiedID p_shaderID)
{
	//	smShaderAttachment holder;
	//	holder.shader=shader;
	//	shaders.checkAndAdd(holder);
	
	shaderEnable.push_back(true);
	shaders.push_back(p_shaderID);
		

}
void smRenderDetail::addVAO(smUnifiedID p_shaderID)
{
	//	smShaderAttachment holder;
	//	holder.shader=shader;
	//	shaders.checkAndAdd(holder);
	
		

	VAOs.push_back(p_shaderID);
	VAOEnable.push_back(true);

}














	
