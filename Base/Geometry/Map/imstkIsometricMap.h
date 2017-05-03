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

#ifndef imstkIsometricMap_h
#define imstkIsometricMap_h

#include "imstkGeometryMap.h"

namespace imstk
{
///
/// \class IsometricMap
///
/// \brief A maps that lets the slave follow the master with a
/// specified offset in position and orientation
///
class IsometricMap : public GeometryMap
{
public:

    ///
    /// \brief Constructor
    ///
    IsometricMap() :
        GeometryMap(GeometryMap::Type::Isometric),
        m_rigidTransform(RigidTransform3d::Identity())
    {}

    ///
    /// \brief Destructor
    ///
    ~IsometricMap() = default;

    ///
    /// \brief Compute the map
    ///
    void compute() override {}

    ///
    /// \brief Apply the map
    ///
    void apply() override;

    ///
    /// \brief Check the validity of the map
    ///
    inline bool isValid() const override
    {
        return true;
    };

    ///
    /// \brief Set the transform
    ///
    void setTransform(const RigidTransform3d& affineTransform);

    ///
    /// \brief Get the transform
    ///
    const RigidTransform3d& getTransform() const;

protected:

    RigidTransform3d m_rigidTransform; ///> Rigid transform
};
} // imstk

#endif // imstkIsometricMap_h
