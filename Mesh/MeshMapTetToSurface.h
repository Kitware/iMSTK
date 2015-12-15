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

#ifndef SM_MESH_MAP_TET_TO_SURFACE
#define SM_MESH_MAP_TET_TO_SURFACE

// SimMedTK includes
#include "Mesh/MeshMap.h"

///
/// \brief Maps a tetrahedral volume mesh (master) to
/// the surface mesh (slave) using custom interpolation
///
class MeshMapTetToSurface : public MeshMap
{

public:
    ///
    /// \brief constructor
    ///
    MeshMapTetToSurface();

    ///
    /// \brief constructor with the two meshes being mapped
    /// as parameters
    ///
    MeshMapTetToSurface(
        std::shared_ptr<Core::BaseMesh>& masterMesh,
        std::shared_ptr<Core::BaseMesh>& slaveMesh);

    ///
    /// \brief destructor
    ///
    ~MeshMapTetToSurface();

    ///
    /// \brief apply the map
    ///
    virtual void apply() override;

    ///
    /// \brief compute the map
    ///
    void compute() override;

private:

    std::vector<int> vertIndices;///> List of indices of vectors

    std::vector<double> weights;///> List of weights
};

#endif //SM_MESH_MAP_TET_TO_SURFACE
