/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkMacros.h"
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
    AnimationObject(const std::string& name = "AnimationObject") : SceneObject(name) { }
    ~AnimationObject() override = default;

    IMSTK_TYPE_NAME(AnimationObject)

    ///
    /// \brief Set/Get animation model
    ///@{
    std::shared_ptr<AnimationModel> getAnimationModel() const;
    void setAnimationModel(std::shared_ptr<AnimationModel> model);
    ///@}

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
} // namespace imstk