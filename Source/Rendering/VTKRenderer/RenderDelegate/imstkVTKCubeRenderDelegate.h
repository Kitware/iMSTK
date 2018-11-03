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

#ifndef imstkVTKCubeRenderDelegate_h
#define imstkVTKCubeRenderDelegate_h

#include <memory>

#include "imstkVTKRenderDelegate.h"
#include "imstkCube.h"

#include "vtkTransformPolyDataFilter.h"

namespace imstk
{
///
/// \class CubeRenderDelegate
///
/// \brief Cube render delegate
///
class VTKCubeRenderDelegate : public VTKRenderDelegate
{
public:
    ///
    /// \brief Constructor
    ///
    VTKCubeRenderDelegate(std::shared_ptr<Cube> cube);

    ///
    /// \brief Destructor
    ///
    ~VTKCubeRenderDelegate() = default;

    ///
    /// \brief Update cube source based on the cube geometry
    ///
    void updateDataSource() override;

    ///
    /// \brief Returns the cube geometry
    ///
    std::shared_ptr<Geometry> getGeometry() const override;

    ///
    /// \brief Return the render material
    ///
    std::shared_ptr<RenderMaterial> getRenderMaterial() const { return m_geometry->getRenderMaterial(); };

protected:
    std::shared_ptr<Cube> m_geometry;   ///> Geometry

    vtkSmartPointer<vtkTransformPolyDataFilter> m_transformFilter; ///> Source
};
} // imstk

#endif // ifndef imstkCubeRenderDelegate_h
