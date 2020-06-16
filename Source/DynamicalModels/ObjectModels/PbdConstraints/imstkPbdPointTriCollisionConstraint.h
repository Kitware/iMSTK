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

#include "imstkPbdCollisionConstraint.h"

namespace imstk
{
///
/// \class PbdPointTriangleConstraint
///
/// \brief The PbdPointTriangleConstraint class for point-triangle collision response
///
class PbdPointTriangleConstraint : public PbdCollisionConstraint
{
public:
    PbdPointTriangleConstraint() : PbdCollisionConstraint(1, 3)
    {}

    ///
    /// \brief Returns the type of the pbd collision constraint
    ///
    Type getType() const
    {
        return Type::PointTriangle;
    }

    ///
    /// \brief initialize constraint
    /// \param pIdx1 index of the point from object1
    /// \param pIdx2 first point of the triangle from object2
    /// \param pIdx3 second point of the triangle from object2
    /// \param pIdx4 third point of the triangle from object2
    /// \return
    ///
    void initConstraint(std::shared_ptr<PbdModel> model1, const size_t& pIdx1,
                        std::shared_ptr<PbdModel> model2, const size_t& pIdx2,
                        const size_t& pIdx3, const size_t& pIdx4);

    ///
    /// \brief
    ///
    bool solvePositionConstraint() override;
};
}
