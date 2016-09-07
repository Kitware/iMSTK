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

#ifndef imstkPlane_h
#define imstkPlane_h

// imstk
#include "imstkGeometry.h"

namespace imstk
{

///
/// \class Plane
///
/// \brief Plane geometry
///
class Plane : public Geometry
{
public:
    ///
    /// \brief Constructor
    ///
    Plane(const Vec3d & position = WORLD_ORIGIN,
          const Vec3d & normal = UP_VECTOR,
          const double& width = 1) :
        Geometry(Geometry::Type::Plane,
                 position,
                 Quatd::FromTwoVectors(UP_VECTOR, normal)),
        m_width(width)
    {}

    ///
    /// \brief Default destructor
    ///
    ~Plane() = default;

    ///
    /// \brief Print the plane info
    ///
    void print() const override;

    ///
    /// \brief Returns the volume (=0.0) for the plane
    ///
    double getVolume() const override;

    // Accessors
    ///
    /// \brief Returns the normal of the plane
    ///
    Vec3d getNormal() const;

    ///
    /// \brief Sets the normal to the plane
    ///
    void setNormal(const Vec3d& normal);

    ///
    /// \brief Returns the width of the plane
    ///
    const double& getWidth() const;

    ///
    /// \brief Sets the width of the plane
    ///
    void setWidth(const double& width);

protected:
    double m_width; ///> Width of the plane (for display)
};

} // imstk

#endif // ifndef imstkPlane_h
