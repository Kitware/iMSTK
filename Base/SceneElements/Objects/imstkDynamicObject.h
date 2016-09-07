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

#ifndef imstkDynamicObject_h
#define imstkDynamicObject_h

#include "imstkSceneObject.h"
#include "imstkDynamicalModel.h"

namespace imstk
{

///
/// \class DynamicObject
///
/// \brief Base class for scene objects that move and/or deform
///
class DynamicObject : public SceneObject
{
public:

    ///
    /// \brief Destructor
    ///
    virtual ~DynamicObject() = default;

    ///
    /// \brief Set/Get the geometry used for Physics computations
    ///
    std::shared_ptr<Geometry> getPhysicsGeometry() const;
    void setPhysicsGeometry(std::shared_ptr<Geometry> geometry);

    ///
    /// \brief Set/Get the Physics-to-Collision map
    ///
    std::shared_ptr<GeometryMap> getPhysicsToCollidingMap() const;
    void setPhysicsToCollidingMap(std::shared_ptr<GeometryMap> map);

    ///
    /// \brief Set/Get the Physics-to-Visual map
    ///
    std::shared_ptr<GeometryMap> getPhysicsToVisualMap() const;
    void setPhysicsToVisualMap(std::shared_ptr<GeometryMap> map);

    ///
    /// \brief Set/Get dynamical model
    ///
    std::shared_ptr<DynamicalModel> getDynamicalModel() const;
    virtual void setDynamicalModel(std::shared_ptr<DynamicalModel> dynaModel);

    ///
    /// \brief Returns the number of degree of freedom
    ///
    size_t getNumOfDOF() const;

protected:

    ///
    /// \brief Constructor
    ///
    DynamicObject(std::string name) : SceneObject(name){}

    std::shared_ptr<DynamicalModel> m_dynamicalModel;           ///> Dynamical model
    std::shared_ptr<Geometry> m_physicsGeometry;                ///> Geometry used for Physics

    //Maps
    std::shared_ptr<GeometryMap> m_physicsToCollidingGeomMap;   ///> Maps from Physics to collision geometry
    std::shared_ptr<GeometryMap> m_physicsToVisualGeomMap;      ///> Maps from Physics to visual geometry

    size_t numDOF; ///> Number of degree of freedom of the body in the discretized model
};

} // imstk

#endif // ifndef imstkDynamicObject_h
