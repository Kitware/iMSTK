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

#ifndef  SMLIGHT_H
#define  SMLIGHT_H

// STL includes
#include <string>

// SimMedTK includes
#include "Config.h"
#include "CoreClass.h"
#include "Rendering/ConfigRendering.h"
#include "Vector.h"

#define SMLIGHT_SPOTMAX 128

class smLight;
/// \brief light type; infinite or spotlight
enum smLightType
{
    SIMMEDTK_LIGHT_SPOTLIGHT,
    SIMMEDTK_LIGHT_INFINITELIGHT
};
/// \brief location of the light type
enum smLightLocationType
{
    SIMMEDTK_LIGHTPOS_EYE,
    SIMMEDTK_LIGHTPOS_WORLD
};
/// \brief light position. light at inifinite position can be defined with w
struct smLightPos
{
public:
    smLightPos(float p_x = 0.0, float p_y = 0.0, float p_z = 0.0, float p_w = 1.0);
    void setPosition(const smVec3d &p)
    {
        position = p;
    }

    const smVec3d &getPosition() const
    {
        return position;
    }

private:
    float w;
    smVec3d position;
    friend smLight;
};

/// \brief Basic light funtionality for the viewer..Be aware that if the light is
///infinite the cutoff angle is still in active. You could see sudden shade in the objects
///if you don't pay attention to the cut-off angle..If you want the scene to be fully lit
///set cut-off angle 180 degrees.
///Also when the params SIMMEDTK_LIGHT_INFINITELIGHT,SIMMEDTK_LIGHTPOS_EYE are used, this means
//the light will be positioned with respect to eye coord and the light will be inifinite
//Therefore, the light should be positioned with a slight offset respect to eye pos(0,0,0);
///such as (0,0.5,0)(like a head lamp)
struct smLight
{

protected:
    bool enabled;
    bool previousState;

public:
    bool drawEnabled;

    int renderUsage;
    std::string name;
    smLightLocationType lightLocationType;
    smLightType lightType;

    float attn_constant;
    float attn_linear;
    float attn_quadratic;

    smLight(std::string p_name = "", smLightType p_lightType = SIMMEDTK_LIGHT_INFINITELIGHT,
            smLightLocationType p_lightLocation = SIMMEDTK_LIGHTPOS_EYE);
    /// \brief set light type
    void setType(smLightType p_lightType);
    /// \brief  returns if the light is enabled or not
    bool isEnabled();
    /// \brief  activate the light
    void activate(bool p_state);
    /// \brief  light properties
    smColor lightColorDiffuse;
    smColor lightColorAmbient;
    smColor lightColorSpecular;

    smLightPos  lightPos;
    //Vec3d direction;
    //between 0-1.0
    /// \brief  higher spot exponents result in a more focused light source,
    //regardless of the spot cutoff angle. default is zero
    float spotExp;
    ///angle between 0-90 and 180 is also accepted
    float spotCutOffAngle;
    /// \brief light direction, up vector, transverse direction, focus point
    smVec3d direction;
    smVec3d upVector;
    smVec3d transverseDir;
    smVec3d focusPosition;//it is for shadow
    /// \brief update light direction
    void updateDirection();
    /// \brief  default direction for light, upvector and transverse direction
    static smVec3d defaultDir;
    static smVec3d defaultUpDir;
    static smVec3d defaultTransDir;

    /// \brief if the light casts shadow, this should be enabled. Unfortunately, we only support one light at a time for shadows
    bool castShadow;
    /// \brief shadow near, far,  aspect ratio and angle
    float shadowNearView;
    float shadowFarView;
    float shadowRatio;
    float shadorAngle;

    static std::shared_ptr<smLight> getDefaultLighting(const std::string &name = "SceneLight")
    {
        std::shared_ptr<smLight>
        light = std::make_shared<smLight>(name,SIMMEDTK_LIGHT_SPOTLIGHT,SIMMEDTK_LIGHTPOS_WORLD);
        light->lightPos.setPosition(smVec3d(10.0, 10.0, 10.0));
        light->lightColorDiffuse.setValue(0.8, 0.8, 0.8, 1);
        light->lightColorAmbient.setValue(0.1, 0.1, 0.1, 1);
        light->lightColorSpecular.setValue(0.9, 0.9, 0.9, 1);
        light->spotCutOffAngle = 60;
        light->direction = smVec3d(0.0, 0.0, -1.0);
        light->drawEnabled = false;
        light->attn_constant = 1.0;
        light->attn_linear = 0.0;
        light->attn_quadratic = 0.0;
        light->activate(true);
        return light;
    }
};

#endif
