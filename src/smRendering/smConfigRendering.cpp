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

#include "smRendering/smConfigRendering.h"
#include "smShader/smShader.h"

smColor smColor::colorWhite(1.0, 1.0, 1.0, 1.0);
smColor smColor::colorBlue(0.0, 0.0, 1.0, 1.0);
smColor smColor::colorGreen(0.0, 1.0, 0.0, 1.0);
smColor smColor::colorRed(1.0, 0.0, 0.0, 1.0);
smColor smColor::colorGray(0.8, 0.8, 0.8, 1.0);
smColor smColor::colorYellow(1, 1, 0, 1);
smColor smColor::colorPink(1, 0, 1, 1);

void smRenderDetail::addShader(smUnifiedID p_shaderID)
{

    shaderEnable.push_back(true);
    shaders.push_back(p_shaderID);
}
void smRenderDetail::addVAO(smUnifiedID p_shaderID)
{

    VAOs.push_back(p_shaderID);
    VAOEnable.push_back(true);
}
smColor::smColor()
{
    rgba[0] = 0.8f;
    rgba[1] = 0.8f;
    rgba[2] = 0.8f;
    rgba[3] = 1.0f;
}
smColor::smColor( float r, float g, float b, float a )
{
    rgba[0] = r;
    rgba[1] = g;
    rgba[2] = b;
    rgba[3] = a;
}
void smColor::darken( float p_darkFactor )
{

    rgba[0] = ( rgba[1] - rgba[1] * ( p_darkFactor ) );
    rgba[1] = ( rgba[2] - rgba[2] * ( p_darkFactor ) );
    rgba[2] = ( rgba[3] - rgba[3] * ( p_darkFactor ) );
    rgba[0] = ( rgba[0] < 0 ? 0 : rgba[0] );
    rgba[1] = ( rgba[1] < 0 ? 0 : rgba[1] );
    rgba[2] = ( rgba[2] < 0 ? 0 : rgba[2] );
}
void smColor::lighten( float p_darkFactor )
{

    rgba[0] = rgba[1] + rgba[1] * ( p_darkFactor );
    rgba[1] = rgba[2] + rgba[2] * ( p_darkFactor );
    rgba[2] = rgba[3] + rgba[3] * ( p_darkFactor );

    rgba[0] = ( rgba[0] > 1.0 ? 1.0 : rgba[0] );
    rgba[1] = ( rgba[1] < 1.0 ? 1.0 : rgba[1] );
    rgba[2] = ( rgba[2] < 1.0 ? 1.0 : rgba[2] );
}
float smColor::operator()( int p_i )
{
    if ( p_i < 0 || p_i > 3 )
    {
        return -1;
    }

    return rgba[p_i];
}
smColor &smColor::operator=( smColor &p_color )
{
    rgba[0] = p_color.rgba[0];
    rgba[1] = p_color.rgba[1];
    rgba[2] = p_color.rgba[2];
    rgba[3] = p_color.rgba[3];
    return *this;
}
GLfloat *smColor::toGLColor()
{
    return ( smGLFloat * )rgba;
}
void smColor::setValue( float p_red, float p_green, float p_blue, float p_alpha )
{
    rgba[0] = p_red;
    rgba[1] = p_green;
    rgba[2] = p_blue;
    rgba[3] = p_alpha;
}
smRenderDetail::smRenderDetail()
{
    renderType = SIMMEDTK_RENDER_MATERIALCOLOR | SIMMEDTK_RENDER_FACES;
    highLightColor.rgba[0] = 1.0f;
    highLightColor.rgba[1] = 0.0f;
    highLightColor.rgba[2] = 0.0f;
    pointSize = 1;
    lineSize = 1;
    shadowColor.rgba[0] = 0.0f;
    shadowColor.rgba[1] = 0.0f;
    shadowColor.rgba[2] = 0.0f;
    shadowColor.rgba[3] = 0.5f;
    colorDiffuse = smColor::colorWhite;
    colorAmbient = smColor::colorWhite;
    colorSpecular = smColor::colorWhite;
    normalColor = smColor::colorGreen;
    wireFrameColor = smColor::colorBlue;
    shininess = 50.0;
    debugDraw = false;
    castShadow = true;
    canGetShadow = true;
}
