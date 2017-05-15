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

#ifndef imstkCylinder_h
#define imstkCylinder_h

// imstk
#include "imstkAnalyticalGeometry.h"

namespace imstk
{
///
/// \class Cylinder
///
/// \brief Cylinder geometry
///
class Cylinder : public AnalyticalGeometry
{
public:
    ///
    /// \brief Constructor
    ///
    Cylinder() : AnalyticalGeometry(Type::Cylinder) {}

    ///
    /// \brief Default destructor
    ///
    ~Cylinder() = default;

    ///
    /// \brief Print the cylinder info
    ///
    void print() const override;

    ///
    /// \brief Returns the volume of the cylinder
    ///
    double getVolume() const override;

    ///
    /// \brief Returns the radius of the cylinder
    ///
    double getRadius(DataType type = DataType::PostTransform);

    ///
    /// \brief Sets the radius of the cylinder
    ///
    void setRadius(const double r);

    ///
    /// \brief Returns the length of the cylinder
    ///
    double getLength(DataType type = DataType::PostTransform);

    ///
    /// \brief Sets the length of the cylinder
    ///
    void setLength(const double r);

protected:
    friend class VTKCylinderRenderDelegate;

    void applyScaling(const double s) override;
    void updatePostTransformData() override;

    double m_radius = 1.0;              ///> Radius of the cylinder
    double m_length = 1.0;              ///> Length of the cylinder
    double m_radiusPostTransform = 1.0; ///> Radius of the cylinder once transform applied
    double m_lengthPostTransform = 1.0; ///> Length of the cylinder once transform applied
};
} // imstk

#endif // ifndef imstkCylinder_h
