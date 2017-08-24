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
#include "imstkAnalyticalGeometry.h"

namespace imstk
{
///
/// \class Capsule
///
/// \brief Capsule geometry
///
class Capsule : public AnalyticalGeometry
{
public:
    ///
    /// \brief Constructor
    ///
    Capsule() : AnalyticalGeometry(Type::Capsule) {}

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

    ///
    /// \brief Returns the radius of the capsule
    ///
    double getRadius(DataType type = DataType::PostTransform);

    ///
    /// \brief Sets the radius of the capsule
    ///
    void setRadius(const double r);

    ///
    /// \brief Returns the length of the capsule
    ///
    double getLength(DataType type = DataType::PostTransform);

    ///
    /// \brief Sets the length of the capsule
    ///
    void setLength(const double l);

protected:
    friend class VTKCapsuleRenderDelegate;

    void applyScaling(const double s) override;
    void updatePostTransformData() override;

    double m_radius = 1.0;              ///> Radius of the hemispheres at the end of the capsule
    double m_radiusPostTransform = 1.0; ///> Radius after transform
    double m_length = 1.0;              ///> Length between the centers of two hemispheres
    double m_lengthPostTransform = 1.0; ///> Length after transform
};
} // imstk

#endif // ifndef imstkCapsule_h
