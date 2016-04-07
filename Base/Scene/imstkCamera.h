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

#ifndef imstkCamera_h
#define imstkCamera_h

#include <string>

#include <vtkSmartPointer.h>
#include <vtkCamera.h>

#include "imstkMath.h"

namespace imstk {

class Camera
{
public:

    Camera() = default;

    ~Camera() = default;

    const Vec3d getPosition() const;
    void setPosition(const Vec3d& p);
    void setPosition(const double& x,
                     const double& y,
                     const double& z);

    const Vec3d getFocalPoint() const;
    void setFocalPoint(const Vec3d& p);
    void setFocalPoint(const double& x,
                       const double& y,
                       const double& z);

    const Vec3d getViewUp() const;
    void setViewUp(const Vec3d& v);
    void setViewUp(const double& x,
                   const double& y,
                   const double& z);

    const double getViewAngle() const;
    void setViewAngle(const double& angle);

    vtkSmartPointer<vtkCamera> getVtkCamera() const;

protected:

    vtkSmartPointer<vtkCamera> m_vtkCamera = vtkSmartPointer<vtkCamera>::New();

};
}

#endif // ifndef imstkCamera_h
