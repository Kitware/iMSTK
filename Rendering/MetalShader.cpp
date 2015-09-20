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
#include "Rendering/MetalShader.h"

#include "Rendering/TextureManager.h"
#include "Core/Event.h"
#include "Core/SDK.h"
#include "Event/KeyboardEvent.h"

MetalShader::MetalShader( const std::string &p_verteShaderFileName,
                              const std::string &p_fragmentFileName ) :
                              Shader(SDK::getInstance()->getErrorLog())
{
    this->log = SDK::getInstance()->getErrorLog();
    this->log->isOutputtoConsoleEnabled = false;
    this->checkErrorEnabled = true;
    setShaderFileName( p_verteShaderFileName, nullptr, p_fragmentFileName );
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

void MetalShader::attachMesh( std::shared_ptr<Core::BaseMesh> p_mesh, char *p_bump, char *p_decal, char *p_specular, char *p_OCC, char *p_disp )
{
    if ( !attachTexture( p_mesh->getUniqueId(), p_bump, "BumpTex" ) )
    {
        std::cout << "Error in bump attachment for mesh:" << p_mesh->getName() << "\n";
    }

    auto id = p_mesh->getUniqueId();
    attachTexture( id, p_decal, "DecalTex" );
    attachTexture( id, p_specular, "SpecularTex" );
    attachTexture( id, p_OCC, "OCCTex" );
    attachTexture( id, p_disp, "DispTex" );
}

void MetalShader::attachMesh( std::shared_ptr<Core::BaseMesh> p_mesh, char *p_bump, char *p_decal, char *p_specular, char *p_OCC, char *p_disp, char *p_alphaMap )
{
    auto id = p_mesh->getUniqueId();
    attachTexture( id, p_bump, "BumpTex" );
    attachTexture( id, p_decal, "DecalTex" );
    attachTexture( id, p_specular, "SpecularTex" );
    attachTexture( id, p_OCC, "OCCTex" );
    attachTexture( id, p_disp, "DispTex" );
    attachTexture( id, p_alphaMap, "AlphaTex" );
}

void MetalShader::draw() const
{
    //placeholder
}

void MetalShader::initDraw()
{
    Shader::initDraw();
    specularPower = this->getFragmentShaderParam( "specularPower" );
    alphaMapGain = this->getFragmentShaderParam( "alphaMapGain" );
    this->tangentAttrib = this->getShaderAtrribParam( "tangent" );
    canGetShadowUniform = getFragmentShaderParam( "canGetShadow" );
}

void MetalShader::predraw(std::shared_ptr<Core::BaseMesh> mesh )
{
    specularPowerValue = mesh->getRenderDetail()->getShininess();
    glUniform1fARB( specularPower, specularPowerValue );
    glUniform1fARB( alphaMapGain, alphaMapGainValue );

    if ( mesh->getRenderDetail()->getCanGetShadow() )
    {
        glUniform1fARB( canGetShadowUniform, 1 );
    }
    else
    {
        glUniform1fARB( canGetShadowUniform, 0 );
    }
}

void MetalShader::handleEvent(std::shared_ptr<core::Event> p_event)
{
    auto keyboardEvent = std::static_pointer_cast<event::KeyboardEvent>(p_event);
    if(keyboardEvent)
    {
        switch(keyboardEvent->getKeyPressed())
        {
            case event::Key::Add:
            {
                specularPowerValue += 5;
                std::cout << specularPowerValue << std::endl;
                break;
            }

            case event::Key::Subtract:
            {
                specularPowerValue -= 5;
                std::cout << specularPowerValue << std::endl;
                break;
            }
            default:
                break;
        }
    }
}

void MetalShader::switchEnable()
{
    //
}

void MetalShader::switchDisable()
{
    //
}

MetalShaderShadow::MetalShaderShadow( const std::string &p_vertexShaderFileName,
                                      const std::string &p_fragmentShaderFileName ) :
    MetalShader( p_vertexShaderFileName, p_fragmentShaderFileName )
{
    createParam( "ShadowMapTEST" );
    createParam( "canGetShadow" );
}

void MetalShaderShadow::initDraw()
{
    MetalShader::initDraw();
    this->print();
    shadowMapUniform = getFragmentShaderParam( "ShadowMapTEST" );
    canGetShadowUniform = getFragmentShaderParam( "canGetShadow" );
}

void MetalShaderShadow::predraw( std::shared_ptr<Core::BaseMesh> p_mesh )
{
    MetalShader::predraw( p_mesh );

    if ( p_mesh->getRenderDetail()->getCanGetShadow() )
    {
        glUniform1fARB( canGetShadowUniform, 1 );
    }
    else
    {
        glUniform1fARB( canGetShadowUniform, 0 );
    }

    TextureManager::activateTexture( "depth", 30, shadowMapUniform );
}

MetalShaderSoftShadow::MetalShaderSoftShadow() :
    MetalShader( "shaders/SingleShadowVertexBumpMap2.cg",
                 "shaders/SingleShadowFragmentBumpMap2.cg" )
{
    createParam( "ShadowMapTEST" );
}

void MetalShaderSoftShadow::initDraw()
{
    MetalShader::initDraw();
    this->print();
    shadowMapUniform = getFragmentShaderParam( "ShadowMapTEST" );
}

void MetalShaderSoftShadow::predraw( std::shared_ptr<Core::BaseMesh> p_mesh )
{
    MetalShader::predraw( p_mesh );
    TextureManager::activateTexture( "depth", 30, shadowMapUniform );
}

void MetalShader::predraw ( std::shared_ptr<SurfaceMesh> ) {}
void MetalShaderSoftShadow::predraw ( std::shared_ptr<SurfaceMesh> ) {}
void MetalShaderShadow::predraw ( std::shared_ptr<SurfaceMesh> ) {}
