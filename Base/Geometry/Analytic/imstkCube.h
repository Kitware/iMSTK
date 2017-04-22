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

#include "imstkAnalyticalGeometry.h"

namespace imstk {

///
/// \class Cube
///
/// \brief Cube geometry
///
class Cube : public AnalyticalGeometry
{
public:

    Cube() : AnalyticalGeometry(Type::Cube) {}

    ~Cube() = default;

    ///
    /// \brief Print the cube info
    ///
    void print() const override;

    ///
    /// \brief Returns the volume of the cube
    ///
    double getVolume() const override;

    ///
    /// \brief Returns the width of the cube
    ///
    double getWidth(DataType type = DataType::PostTransform);

    ///
    /// \brief Sets the width of the cube
    ///
    void setWidth(const double w);

protected:
    friend class VTKCubeRenderDelegate;

    void applyScaling(const double s) override;
    void updatePostTransformData() override;

    double m_width = 1.0;               ///> Width of the cube
    double m_widthPostTransform = 1.0;  ///> Width of the cube once transform applied

};
}

#endif // ifndef imstkCube_h
