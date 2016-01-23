// This file is part of the iMSTK project.
//
// Copyright (c) Kitware, Inc.
//
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

// iMSTK includes
#include "Core/Light.h"
#include "Core/Quaternion.h"

core::Vec3d Light::defaultDir(0, 0, -1.0);
core::Vec3d Light::defaultUpDir(0, 1, 0.0);
core::Vec3d Light::defaultTransDir(1, 0, 0.0);

void Light::updateDirection()
{
    double angle;
    core::Vec3d dirNorm = direction.normalized();

    angle = std::acos(dirNorm.dot(defaultDir));
    core::Vec3d axisOfRot = dirNorm.cross(defaultDir).normalized();

    core::Quaterniond rot = getRotationQuaternion(-angle,axisOfRot);

    upVector = rot*defaultUpDir;
    transverseDir = rot*defaultTransDir;
}

LightPos::LightPos( float p_x, float p_y, float p_z, float /*p_w*/ )
{
    position << p_x, p_y, p_z;
}

Light::Light( std::string p_name, LightType p_lightType, LightLocationType p_lightLocation )
{
    name = p_name;
    enabled = false;
    previousState = false;
    lightPos.setPosition(core::Vec3d::Zero());

    if ( p_lightType == InfiniteLight )
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
    drawEnabled = false;
    castShadow = false;
    shadowNearView = 0.01;
    shadowFarView = 4000;
    shadowRatio = 1.0;
    shadorAngle = 60;
    attn_constant = 1.0;
    attn_linear = 0.0;
    attn_quadratic = 0.0;
}
void Light::setType( LightType p_lightType )
{
    if ( p_lightType == InfiniteLight )
    {
        lightPos.w = 0.0;
    }
    else
    {
        lightPos.w = 1.0;
    }
}
bool Light::isEnabled()
{
    return enabled;
}
void Light::activate( bool p_state )
{
    enabled = p_state;
    previousState = enabled;
}
