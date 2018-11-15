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

#ifndef imstkPickingCH_h
#define imstkPenaltyCH_h

// std library
#include <memory>

// imstk
#include "imstkCollisionHandling.h"
#include "imstkNonlinearSystem.h"

namespace imstk
{
class CollidingObject;
class DeformableObject;
class CollisionData;

///
/// \class PickingCH
///
/// \brief Implements nodal picking
///
class PickingCH : public CollisionHandling
{
public:

    ///
    /// \brief Constructor
    ///
    PickingCH(const Side& side,
              const CollisionData& colData,
              std::shared_ptr<DeformableObject> obj) :
        CollisionHandling(Type::NodalPicking, side, colData),
        m_object(obj){}

    PickingCH() = delete;

    ///
    /// \brief Destructor
    ///
    ~PickingCH() = default;

    ///
    /// \brief Compute forces based on collision data
    ///
    void processCollisionData() override;

    ///
    /// \brief Add LPC constraints for the node that is picked
    ///
    void addPickConstraints(std::shared_ptr<DeformableObject> deformableObj);

    ///
    /// \brief Get the vector denoting the filter
    ///
    void setDynamicLinearProjectors(std::vector<LinearProjectionConstraint>* f)
    {
        m_DynamicLinearProjConstraints = f;
    }

    ///
    /// \brief Get the vector denoting the filter
    ///
    std::vector<LinearProjectionConstraint>& getDynamicLinearProjectors()
    {
        return *m_DynamicLinearProjConstraints;
    }

private:

    std::shared_ptr<DeformableObject> m_object;     ///> Deformable object
    std::vector<LinearProjectionConstraint>  *m_DynamicLinearProjConstraints;
};
}

#endif // ifndef imstkPenaltyCH_h