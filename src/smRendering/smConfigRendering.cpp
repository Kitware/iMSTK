#include "smRendering/smConfigRendering.h"
#include "smShader/smShader.h"

smColor smColor::colorWhite(1.0,1.0,1.0,1.0);
smColor smColor::colorBlue (0.0,0.0,1.0,1.0);
smColor smColor::colorGreen(0.0,1.0,0.0,1.0);
smColor smColor::colorRed  (1.0,0.0,0.0,1.0);
smColor smColor::colorGray (0.8,0.8,0.8,1.0);
smColor smColor::colorYellow(1,1,0,1);
smColor smColor::colorPink(1,0,1,1);

void smRenderDetail::addShader(smUnifiedID p_shaderID){

	shaderEnable.push_back(true);
	shaders.push_back(p_shaderID);
}
void smRenderDetail::addVAO(smUnifiedID p_shaderID){

	VAOs.push_back(p_shaderID);
	VAOEnable.push_back(true);
}
