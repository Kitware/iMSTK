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

#include "imstkVisualModel.h"
#include "imstkSceneEntity.h"

namespace imstk
{
class Geometry;
class DeviceClient;

///
/// \class SceneObject
///
/// \brief Base class for all scene objects. A scene object can optionally be visible and
/// collide with other scene objects. A object of the class is static.
///
class SceneObject : public SceneEntity
{
public:
    enum class Type
    {
        Visual,
        Animation,
        Colliding,
        Rigid,
        FEMDeformable,
        Pbd,
        SPH
    };

    ///
    /// \brief Constructor
    ///
    explicit SceneObject(const std::string& name);

    ///
    /// \brief Destructor
    ///
    virtual ~SceneObject() = default;

    ///
    /// \brief Get the type of the object
    ///
    const Type& getType() const;

    ///
    /// \brief Get/Set the custom name of the scene object
    ///
    const std::string& getName() const;
    void setName(const std::string& name);

    ///
    /// \brief DEPRECATED: Get/Set geometry used for viewing
    ///
    /// Use getVisualModel() and addVisualModel() instead
    ///
    std::shared_ptr<Geometry> getVisualGeometry() const;
    void setVisualGeometry(std::shared_ptr<Geometry> geometry);

    ///
    /// \brief Get/add visual model
    ///
    std::shared_ptr<VisualModel> getVisualModel(unsigned int index);
    void addVisualModel(std::shared_ptr<VisualModel> visualModel);

    ///
    /// \brief Get all visual models
    ///
    const std::vector<std::shared_ptr<VisualModel>>& getVisualModels();

    ///
    /// \brief Get number of visual models
    ///
    size_t getNumVisualModels();

    ///
    /// \brief Get the master geometry
    ///
    virtual std::shared_ptr<Geometry> getMasterGeometry() const;

    ///
    /// \brief
    ///
    virtual void updateGeometries() {}

    ///
    /// \brief Initialize the scene object
    ///
    virtual bool initialize() { return true; }

    ///
    /// \brief
    ///
    virtual void reset() {}

protected:
    ///
    /// \brief Assigns the type of the object
    ///
    void setType(Type type);

    Type m_type;                                              ///> Type of the scene object
    std::string m_name;                                       ///> Custom name of the scene object
    std::vector<std::shared_ptr<VisualModel>> m_visualModels; ///> Visual objects for rendering
};

using VisualObject = SceneObject;
} // imstk
