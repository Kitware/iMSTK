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

// STD includes
#include <cmath>

// SimMedTK includes
#include "smRendering/smLight.h"
#include "smUtilities/smQuaternion.h"

smVec3d smLight::defaultDir(0, 0, -1.0);
smVec3d smLight::defaultUpDir(0, 1, 0.0);
smVec3d smLight::defaultTransDir(1, 0, 0.0);

void smLight::updateDirection()
{
    double angle;
    smVec3d dirNorm = direction.normalized();

    angle = std::acos(dirNorm.dot(defaultDir));
    smVec3d axisOfRot = dirNorm.cross(defaultDir).normalized();

    smQuaterniond rot = getRotationQuaternion(-angle,axisOfRot);

    upVector = rot*defaultUpDir;
    transverseDir = rot*defaultTransDir;
}

smLightPos::smLightPos( float p_x, float p_y, float p_z, float /*p_w*/ )
{
    position << p_x, p_y, p_z;
}

smLight::smLight( std::string p_name, smLightType p_lightType, smLightLocationType p_lightLocation )
{
    name = p_name;
    enabled = false;
    previousState = false;
    lightPos.setPosition(smVec3d::Zero());

    if ( p_lightType == SIMMEDTK_LIGHT_INFINITELIGHT )
    {
        lightPos.w = 0.0;
    }
    else
    {
        lightPos.w = 1.0;
    }

    lightType = p_lightType;
    lightLocationType = p_lightLocation;

    direction << 0, 0, -1.0;
    upVector = defaultUpDir;
    transverseDir = defaultTransDir;
    focusPosition << 0, 0, 0;

    spotCutOffAngle = 45.0;
    spotExp = 0.0;
    lightColorAmbient.setValue( 0.2, 0.2, 0.2, 1.0 );
    lightColorDiffuse.setValue( 0.8f, 0.8f, 0.8, 1.0f );
    lightColorSpecular.setValue( 0.5f, 0.5f, 0.5f, 1.0f );
    drawEnabled = true;
    castShadow = false;
    shadowNearView = 0.01;
    shadowFarView = 4000;
    shadowRatio = 1.0;
    shadorAngle = 60;
    attn_constant = 1.0;
    attn_linear = 0.0;
    attn_quadratic = 0.0;
}
void smLight::setType( smLightType p_lightType )
{
    if ( p_lightType == SIMMEDTK_LIGHT_INFINITELIGHT )
    {
        lightPos.w = 0.0;
    }
    else
    {
        lightPos.w = 1.0;
    }
}
bool smLight::isEnabled()
{
    return enabled;
}
void smLight::activate( bool p_state )
{
    enabled = p_state;
    previousState = enabled;
}
