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

#ifndef imstkSphere_h
#define imstkSphere_h

#include "imstkGeometry.h"

namespace imstk {
class Sphere : public Geometry
{
public:

    Sphere(const Vec3d & position = WORLD_ORIGIN,
           const double& radius = 10) :
        Geometry(GeometryType::Sphere,
                 position,
                 Quatd::Identity()),
        m_radius(radius)
    {}

    ~Sphere() = default;

    const double& getRadius() const;
    void          setRadius(const double& radius);

protected:

    double m_radius;
};
}

#endif // ifndef imstkSphere_h
