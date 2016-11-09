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

#ifndef imstkPbdCollisionConstraint_h
#define imstkPbdCollisionConstraint_h

#include "imstkPbdModel.h"

#include <vector>

namespace imstk
{

class PbdModel;

///
/// \class PbdCollisionConstraint
///
/// \brief
///
class PbdCollisionConstraint
{
public:
    enum class Type
    {
        EdgeEdge,
        PointTriangle
    };

    ///
    /// \brief
    ///
    PbdCollisionConstraint(const unsigned int& n1, const unsigned int& n2)
    {
        m_bodiesFirst.resize(n1);
        m_bodiesSecond.resize(n2);
    }

    ///
    /// \brief
    ///
    virtual bool solvePositionConstraint()
    {
        return true;
    }

public:
    std::vector<size_t> m_bodiesFirst;    ///> index of points for the first object
    std::vector<size_t> m_bodiesSecond;   ///> index of points for the second object

    std::shared_ptr<PbdModel> m_model1;
    std::shared_ptr<PbdModel> m_model2;
};

}

#endif // imstkPbdCollisionConstraint_h