/*=========================================================================

   Library: iMSTK

   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
   & Imaging in Medicine, Rensselaer Polytechnic Institute.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

=========================================================================*/

#pragma once

#include "imstkSceneObject.h"

namespace imstk
{
class Geometry;
class AnimationModel;

///
/// \brief TODO
///
class AnimationObject : public SceneObject
{
public:
    ///
    /// \brief Constructor
    ///
    explicit AnimationObject(const std::string& name) : SceneObject(name)
    {
        m_type = Type::Animation;
    }

    ///
    /// \brief Default destructor
    ///
    virtual ~AnimationObject() = default;

    ///
    /// \brief Set/get animation model
    ///
    std::shared_ptr<AnimationModel> getAnimationModel() const;
    void setAnimationModel(std::shared_ptr<AnimationModel> model);

    ///
    /// \brief Initialize the scene object
    ///
    virtual bool initialize() override
    {
        if (SceneObject::initialize())
        {
            return true;
        }
        else
        {
            return false;
        }
    }

protected:
    std::shared_ptr<AnimationModel> m_animationModel;
};
} // imstk
