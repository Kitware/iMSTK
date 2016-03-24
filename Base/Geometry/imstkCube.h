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

#ifndef imstkCube_h
#define imstkCube_h

#include "imstkGeometry.h"

namespace imstk {
class Cube : public Geometry
{
public:

    Cube(const Vec3d & position = ORIGIN,
         const double& width = 10) :
        Geometry(GeometryType::Cube,
                 position,
                 Quatd::Identity()),
        m_width(width)
    {}

    ~Cube() = default;

    const double& getWidth() const;
    void          setWidth(const double& width);

protected:

    double m_width;
};
}

#endif // ifndef imstkCube_h
