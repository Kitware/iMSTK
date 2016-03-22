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

#ifndef imstkAnalyticalGeometry_h
#define imstkAnalyticalGeometry_h

#include "imstkGeometry.h"

namespace imstk {
class AnalyticalGeometry : public Geometry
{
public:

    ~AnalyticalGeometry() = default;

    void         translate(const Vec3d& t) override;
    void         rotate(const Quatd& r) override;
    void         rotate(const Vec3d & axis,
                        const double& angle) override;

    const Vec3d& getPosition() const;
    void         setPosition(const Vec3d& position);

    const Quatd& getOrientation() const;
    void         setOrientation(const Quatd& orientation);

protected:

    AnalyticalGeometry(GeometryType type,
                       const Vec3d& position = Vec3d(),
                       const Quatd& orientation = Quatd()) :
        Geometry(type),
        m_position(position),
        m_orientation(orientation)
    {}

    Vec3d m_position;
    Quatd m_orientation;
};
}

#endif // ifndef imstkAnalyticalGeometry_h
