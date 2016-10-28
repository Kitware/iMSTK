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

#ifndef imstkSceneObject_h
#define imstkSceneObject_h

#include <memory>
#include <string>

namespace imstk
{

class Geometry;
class SceneObjectController;
class DeviceClient;

///
/// \class SceneObject
///
/// \brief Base class for all scene objects. A scene object can optionally be visible and
/// collide with other scene objects. A object of the class is static.
///
class SceneObject
{
public:
    enum class Type
    {
        Visual,
        Colliding,
        Rigid,
        Deformable,
        Pbd
    };

    ///
    /// \brief Constructor
    ///
    SceneObject(std::string name) : m_name(name)
    {
        m_type = Type::Visual;
    }

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
    /// \brief Get/Set geometry used for viewing
    ///
    std::shared_ptr<Geometry> getVisualGeometry() const;
    void setVisualGeometry(std::shared_ptr<Geometry> geometry);

    ///
    /// \brief Get the master geometry
    ///
    virtual std::shared_ptr<Geometry> getMasterGeometry() const;

    ///
    /// \brief Get the object controller
    ///
    std::shared_ptr<SceneObjectController> getController() const;

    ///
    /// \brief Setup a controller for the object for a given device client
    ///
    std::shared_ptr<SceneObjectController> setupController(std::shared_ptr<DeviceClient> deviceClient);

    ///
    /// \brief
    ///
    bool isVisible() const { return true; };

    ///
    /// \brief
    ///
    virtual bool isCollidable() const { return false; };

    ///
    /// \brief
    ///
    virtual bool isPhysical() const { return false; };

protected:
    ///
    /// \brief Assigns the type of the object
    ///
    void setType(Type type);

    Type m_type; ///> Type of the scene object
    std::string m_name; ///> Custom name of the scene object
    std::shared_ptr<Geometry> m_visualGeometry; ///> Geometry for rendering
    std::shared_ptr<SceneObjectController> m_controller; ///> Object controller
 };

using VisualObject = SceneObject;

} // imstk

#endif // ifndef imstkSceneObject_h
