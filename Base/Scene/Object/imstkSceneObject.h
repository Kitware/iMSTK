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

namespace imstk {

class Geometry;

class SceneObject
{
public:

    enum class Type
    {
        Visual,
        Static,
        VirtualCoupled,
        Rigid,
        Deformable
    };

    SceneObject(std::string name) : m_name(name) {}

    ~SceneObject() = default;

    const Type& getType() const;

    const std::string& getName() const;
    void setName(std::string name);

    std::shared_ptr<Geometry> getVisualGeometry() const;
    void setVisualGeometry(std::shared_ptr<Geometry> geometry);

protected:

    void setType(Type type);

    Type m_type = Type::Visual;
    std::string m_name;
    std::shared_ptr<Geometry> m_visualGeometry; ///> Geometry for rendering
};

using VisualObject = SceneObject;
}

#endif // ifndef imstkSceneObject_h
