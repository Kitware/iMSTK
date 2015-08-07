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

#ifndef SMVOLUMEMESH_H
#define SMVOLUMEMESH_H

// SimMedTK includes
#include "Core/BaseMesh.h"

/// \brief this class is derived from generic Mesh class. Tetrahedron are building blocks of this volume mesh.
///  It also retains the surface triangle structure for rendering and collision detection purposes.
///  This surface triangle structure might be extracted from the volume mesh while loading
class VolumeMesh: public Core::BaseMesh
{
public:

};

#endif
