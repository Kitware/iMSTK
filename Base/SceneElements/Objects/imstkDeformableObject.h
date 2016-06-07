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

#ifndef imstkDeformableObject_h
#define imstkDeformableObject_h

#include <memory>
#include <string>

#include "imstkDynamicObject.h"
#include "imstkDynamicalModel.h"

#include "imstkKinematicState.h"
#include "imstkMath.h"

namespace imstk {

///
/// \class DeformableObject
///
/// \brief Scene objects that can deform
///
class DeformableObject : public DynamicObject
{
public:

    ///
    /// \brief Constructor
    ///
    DeformableObject(std::string name) : DynamicObject(name)
    {
        m_type = SceneObject::Type::Deformable;
    }

    ///
    /// \brief Destructor
    ///
    ~DeformableObject() = default;

    ///
    /// \brief Initialize the kinematic state of the body
    ///
    void initializeState();
    void initializeState(const Vectord& p, const Vectord& v);

    // Get/Set States of the body
    const Vectord& getDisplacements() const;
    const Vectord& getPrevDisplacements() const;
    const Vectord& getVelocities() const;
    const Vectord& getPrevVelocities() const;

protected:

    std::shared_ptr<DynamicalModel> m_dynamicalModel; ///> Dynamical model
};

}

#endif // ifndef imstkDeformableObject_h
