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

#ifndef  CORE_LIGHT_H
#define  CORE_LIGHT_H

// STL includes
#include <string>

// iMSTK includes
#include "Core/Config.h"
#include "Core/CoreClass.h"
#include "Core/Vector.h"
#include "Core/ConfigRendering.h"

#define IMSTK_LIGHT_SPOTMAX 128

class Light;
/// \brief light position. light at inifinite position can be defined with w
struct LightPos
{
public:

    LightPos(float p_x = 0.0, float p_y = 0.0, float p_z = 0.0, float p_w = 1.0);
    void setPosition(const core::Vec3d &p)
    {
        position = p;
    }

    const core::Vec3d &getPosition() const
    {
        return position;
    }

private:
    float w;
    core::Vec3d position;
    friend Light;
};

/// \brief Basic light funtionality for the viewer..Be aware that if the light is
///infinite the cutoff angle is still in active. You could see sudden shade in the objects
///if you don't pay attention to the cut-off angle..If you want the scene to be fully lit
///set cut-off angle 180 degrees.
///Also when the params InfiniteLight,Eye are used, this means
//the light will be positioned with respect to eye coord and the light will be inifinite
//Therefore, the light should be positioned with a slight offset respect to eye pos(0,0,0);
///such as (0,0.5,0)(like a head lamp)
struct Light
{
public:
    /// \brief light type; infinite or spotlight
    enum LightType
    {
        Spotlight,
        InfiniteLight
    };
    /// \brief location of the light type
    enum LightLocationType
    {
        Eye,
        World
    };
protected:
    bool enabled;
    bool previousState;

public:
    bool drawEnabled;

    int renderUsage;
    std::string name;
    LightLocationType lightLocationType;
    LightType lightType;

    float attn_constant;
    float attn_linear;
    float attn_quadratic;

    Light(std::string p_name = "", LightType p_lightType = InfiniteLight,
          LightLocationType p_lightLocation = Eye);
    /// \brief set light type
    void setType(LightType p_lightType);
    /// \brief  returns if the light is enabled or not
    bool isEnabled();
    /// \brief  activate the light
    void activate(bool p_state);
    /// \brief  light properties
    Color lightColorDiffuse;
    Color lightColorAmbient;
    Color lightColorSpecular;

    LightPos  lightPos;

    //between 0-1.0
    /// \brief  higher spot exponents result in a more focused light source,
    //regardless of the spot cutoff angle. default is zero
    float spotExp;
    ///angle between 0-90 and 180 is also accepted
    float spotCutOffAngle;
    /// \brief light direction, up vector, transverse direction, focus point
    core::Vec3d direction;
    core::Vec3d upVector;
    core::Vec3d transverseDir;
    core::Vec3d focusPosition;//it is for shadow
    /// \brief update light direction
    void updateDirection();
    /// \brief  default direction for light, upvector and transverse direction
    static core::Vec3d defaultDir;
    static core::Vec3d defaultUpDir;
    static core::Vec3d defaultTransDir;

    /// \brief if the light casts shadow, this should be enabled. Unfortunately, we only support one light at a time for shadows
    bool castShadow;
    /// \brief shadow near, far,  aspect ratio and angle
    float shadowNearView;
    float shadowFarView;
    float shadowRatio;
    float shadorAngle;

    static std::shared_ptr<Light> getDefaultLighting(const std::string &name = "SceneLight")
    {
        std::shared_ptr<Light>
        light = std::make_shared<Light>(name, Spotlight, World);
        light->lightPos.setPosition(core::Vec3d(10.0, 10.0, 10.0));
        light->lightColorDiffuse.setValue(0.8, 0.8, 0.8, 1);
        light->lightColorAmbient.setValue(0.1, 0.1, 0.1, 1);
        light->lightColorSpecular.setValue(0.9, 0.9, 0.9, 1);
        light->spotCutOffAngle = 60;
        light->direction = core::Vec3d(0.0, 0.0, -1.0);
        light->drawEnabled = false;
        light->attn_constant = 1.0;
        light->attn_linear = 0.0;
        light->attn_quadratic = 0.0;
        light->activate(true);
        return light;
    }
};

#endif
