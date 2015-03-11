// This file is part of the SimMedTK project.
// Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
//                        Rensselaer Polytechnic Institute
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//---------------------------------------------------------------------------
//
// Authors:
//
// Contact:
//---------------------------------------------------------------------------

// SimMedTK includes
#include "smShader/smMetalShader.h"
#include "smCore/smSDK.h"

smMetalShader::smMetalShader( char *p_verteShaderFileName, char *p_fragmentFileName )
{
    this->log = smSDK::getErrorLog();
    this->log->isOutputtoConsoleEnabled = false;
    this->checkErrorEnabled = true;
    setShaderFileName( p_verteShaderFileName, NULL, p_fragmentFileName );
    createParam( "DecalTex" );
    createParam( "BumpTex" );
    createParam( "SpecularTex" );
    createParam( "DispTex" );
    createParam( "OCCTex" );
    createParam( "noiseTex" );
    createParam( "specularPower" );
    createParam( "alphaMap" );
    createParam( "alphaMapGain" );
    createParam( "canGetShadow" );
    createAttrib( "tangent" );

    //if the objtets are static we cannot change this value during runtime
    specularPowerValue = 5.0;
    alphaMapGain = -1;
    alphaMapGainValue = 1.0;
    this->checkErrorEnabled = true;
    log->isOutputtoConsoleEnabled = true;
}
void smMetalShader::attachMesh( smMesh *p_mesh, char *p_bump, char *p_decal, char *p_specular, char *p_OCC, char *p_disp )
{
    if ( !attachTexture( p_mesh->uniqueId, p_bump, "BumpTex" ) )
    {
        std::cout << "Error in bump attachment for mesh:" << p_mesh->name << "\n";
    }

    attachTexture( p_mesh->uniqueId, p_decal, "DecalTex" );
    attachTexture( p_mesh->uniqueId, p_specular, "SpecularTex" );
    attachTexture( p_mesh->uniqueId, p_OCC, "OCCTex" );
    attachTexture( p_mesh->uniqueId, p_disp, "DispTex" );
}
void smMetalShader::attachMesh( smMesh *p_mesh, char *p_bump, char *p_decal, char *p_specular, char *p_OCC, char *p_disp, char *p_alphaMap )
{
    attachTexture( p_mesh->uniqueId, p_bump, "BumpTex" );
    attachTexture( p_mesh->uniqueId, p_decal, "DecalTex" );
    attachTexture( p_mesh->uniqueId, p_specular, "SpecularTex" );
    attachTexture( p_mesh->uniqueId, p_OCC, "OCCTex" );
    attachTexture( p_mesh->uniqueId, p_disp, "DispTex" );
    attachTexture( p_mesh->uniqueId, p_alphaMap, "AlphaTex" );
}
void smMetalShader::draw( smDrawParam p_param )
{
    //placeholder
}
void smMetalShader::initDraw( smDrawParam p_param )
{
    smShader::initDraw( p_param );
    specularPower = this->getFragmentShaderParam( "specularPower" );
    alphaMapGain = this->getFragmentShaderParam( "alphaMapGain" );
    this->tangentAttrib = this->getShaderAtrribParam( "tangent" );
    canGetShadowUniform = getFragmentShaderParam( "canGetShadow" );
}
void smMetalShader::predraw( smMesh *mesh )
{
    specularPowerValue = mesh->renderDetail.shininess;
    glUniform1fARB( specularPower, specularPowerValue );
    glUniform1fARB( alphaMapGain, alphaMapGainValue );

    if ( mesh->renderDetail.canGetShadow )
    {
        glUniform1fARB( canGetShadowUniform, 1 );
    }
    else
    {
        glUniform1fARB( canGetShadowUniform, 0 );
    }
}
void smMetalShader::handleEvent( smEvent *p_event )
{
    smKeyboardEventData *keyBoardData;

    switch ( p_event->eventType.eventTypeCode )
    {
        case SIMMEDTK_EVENTTYPE_KEYBOARD:
            keyBoardData = ( smKeyboardEventData * )p_event->data;

            if ( keyBoardData->keyBoardKey == smKey::Add )
            {
                specularPowerValue += 5;
                std::cout << specularPowerValue << "\n";
            }

            if ( keyBoardData->keyBoardKey == smKey::Subtract )
            {
                specularPowerValue -= 5;
                std::cout << specularPowerValue << "\n";
            }

            break;
    }
}
void smMetalShader::switchEnable()
{
    //
}
void smMetalShader::switchDisable()
{
    //
}
MetalShaderShadow::MetalShaderShadow( char *p_vertexShaderFileName, char *p_fragmentShaderFileName ) :
    smMetalShader( p_vertexShaderFileName, p_fragmentShaderFileName )
{
    createParam( "ShadowMapTEST" );
    createParam( "canGetShadow" );
}
void MetalShaderShadow::initDraw( smDrawParam p_param )
{
    smMetalShader::initDraw( p_param );
    this->print();
    shadowMapUniform = getFragmentShaderParam( "ShadowMapTEST" );
    canGetShadowUniform = getFragmentShaderParam( "canGetShadow" );
}
void MetalShaderShadow::predraw( smMesh *p_mesh )
{
    smMetalShader::predraw( p_mesh );

    if ( p_mesh->renderDetail.canGetShadow )
    {
        glUniform1fARB( canGetShadowUniform, 1 );
    }
    else
    {
        glUniform1fARB( canGetShadowUniform, 0 );
    }

    smTextureManager::activateTexture( "depth", 30, shadowMapUniform );
}
MetalShaderSoftShadow::MetalShaderSoftShadow() :
    smMetalShader( "shaders/SingleShadowVertexBumpMap2.cg",
                 "shaders/SingleShadowFragmentBumpMap2.cg" )
{
    createParam( "ShadowMapTEST" );
}
void MetalShaderSoftShadow::initDraw( smDrawParam p_param )
{
    smMetalShader::initDraw( p_param );
    this->print();
    shadowMapUniform = getFragmentShaderParam( "ShadowMapTEST" );
}
void MetalShaderSoftShadow::predraw( smMesh *p_mesh )
{
    smMetalShader::predraw( p_mesh );
    smTextureManager::activateTexture( "depth", 30, shadowMapUniform );
}
