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

#ifndef imstkCapsule_h
#define imstkCapsule_h

// imstk
#include "imstkGeometry.h"

namespace imstk
{

///
/// \class Capsule
///
/// \brief Capsule geometry
///
class Capsule : public Geometry
{
public:
    ///
    /// \brief Constructor
    ///
    Capsule(const Vec3d& position = WORLD_ORIGIN,
            const double& radius = 1.,
            const double& height = 1.) :
            m_radius(radius),
            m_height(height),
            Geometry(Geometry::Type::Capsule,
                     position,
                     Quatd::Identity()){}

    ///
    /// \brief Default destructor
    ///
    ~Capsule() = default;

    ///
    /// \brief Print the capsule info
    ///
    void print() const override;

    ///
    /// \brief Returns the volume of the capsule
    ///
    double getVolume() const override;

    // Accessors
    ///
    /// \brief Returns the radius of the capsule
    ///
    const double& getRadius() const{ return m_radius; };

    ///
    /// \brief Sets the radius of the capsule
    ///
    void setRadius(const double& r);

    ///
    /// \brief Returns the height of the capsule
    ///
    const double& getHeight() const { return m_height; }

    ///
    /// \brief Sets the height of the capsule
    ///
    void setHeight(const double& h);

protected:

    double m_radius; ///> Radius of the hemispheres at the end of the capsule
    double m_height; ///> Length between the centers of two hemispheres
};

} // imstk

#endif // ifndef imstkCapsule_h