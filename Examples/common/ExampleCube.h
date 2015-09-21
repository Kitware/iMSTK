// This file is part of the SimMedTK project.
// Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
//                        Rensselaer Polytechnic Institute
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//---------------------------------------------------------------------------
//
// Authors:
//
// Contact:
//---------------------------------------------------------------------------

#ifndef MSTK_EXAMPLES_COMMON_EXAMPLECUBE_H
#define MSTK_EXAMPLES_COMMON_EXAMPLECUBE_H

#include <memory>

#include "Core/StaticSceneObject.h"
#include "Geometry/MeshModel.h"

namespace mstk {
namespace Examples {
namespace Common {

/// \brief A simple interface for creating a textured cube in 3D space
///
class ExampleCube
{
public:
    ExampleCube(bool vtkRenderer = true);

    /// \brief Tell the cube to initialize using the VTK renderer
    ///
    void useVTKRenderer(bool b);

    /// \brief Create the cube object, load meshes and textures, etc
    ///
    void setup();

    /// \brief Get a pointer to the cube's scene object
    ///
    /// \return Shared pointer to the internal static scene object
    std::shared_ptr<StaticSceneObject> getStaticSceneObject();

private:
    std::shared_ptr<StaticSceneObject> cube; ///< Cube scene object
    std::shared_ptr<RenderDetail> renderDetail; ///< Details of how to render
    std::shared_ptr<MeshModel> cubeModel; ///< Cube 3D mesh

    bool useVTK; ///< To use, or not use the VTK renderer
};

} //Common
} //Examples
} //mstk

#endif
