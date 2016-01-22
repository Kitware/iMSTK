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

#ifndef SM_MESH_MAP
#define SM_MESH_MAP

// SimMedTK includes
#include "Core/BaseMesh.h"

///
/// \brief Base class to map one mesh to another
///
class MeshMap
{

public:
    ///
    /// \brief Constructor
    ///
    MeshMap();

    ///
    /// \brief Constructor with the two meshes being mapped
    /// as parameters
    ///
    MeshMap(std::shared_ptr<Core::BaseMesh> masterMesh,
            std::shared_ptr<Core::BaseMesh> slaveMesh);

    ///
    /// \brief Destructor
    ///
    ~MeshMap();

    ///
    /// \brief Assign the master mesh
    ///
    void setMasterMesh(std::shared_ptr<Core::BaseMesh> masterMesh);

    ///
    /// \brief Assign the slave mesh
    ///
    void setSlaveMesh(std::shared_ptr<Core::BaseMesh> slaveMesh);

    ///
    /// \brief Returns the master mesh
    ///
    std::shared_ptr<Core::BaseMesh> getMasterMesh();

    ///
    /// \brief Returns the slave mesh
    ///
    std::shared_ptr<Core::BaseMesh> getSlaveMesh();

    ///
    /// \brief apply a given map with given id from the list
    ///
    virtual void apply() = 0;

	///
	/// \brief compute the map
	///
    virtual void computeMap() = 0;

    ///
    /// \brief Deactivate the map
    ///
    void deactivate();

    ///
    /// \brief Activate the map
    ///
    void activate();

	///
	/// \brief Returns true if the map is active, false otherwise
	///
    bool isActive();

protected:
    bool active;///> map is not applied when mute is true

    // Mesh from which the nodal coordinates are interpolated
    std::shared_ptr<Core::BaseMesh> masterMesh;

    // Mesh to which the nodal coordinates are interpolated
    std::shared_ptr<Core::BaseMesh> slaveMesh;
};

#endif //SM_MESH_MAP
