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

#ifndef SM_MESH_MAP_AFFINE
#define SM_MESH_MAP_AFFINE

// SimMedTK includes
#include "Mesh/MeshMap.h"
#include <Eigen/Geometry>

///
/// \brief Map that is a on-to-one map. In other words the slave mesh is got by
/// an affine transform of the master mesh
///
class MeshMapAffine : public MeshMap
{
public:
    using TransformType = Eigen::Transform<double, 3, Eigen::Isometry>;

public:

    ///
    /// \brief Default constructor with affine transform as parameter
    ///
    MeshMapAffine(const TransformType& M);

    ///
    /// \brief constructor with the two meshes being mapped
    /// and the affine transform as parameters
    ///
    MeshMapAffine(
        std::shared_ptr<Core::BaseMesh>& masterMesh,
        std::shared_ptr<Core::BaseMesh>& slaveMesh,
        const TransformType& M);

    ///
    /// \brief destructor
    ///
    ~MeshMapAffine();

    ///
    /// \brief apply the map
    ///
    virtual void apply() override;

    ///
    /// \brief compute the map
    ///
    void compute() override;

	///
	/// \brief Returns the affine transform used for the mapping
	///
    const TransformType& getAffineTransform() const;

private:
    TransformType affineTransform;///> affine transform
};

#endif //SM_MESH_MAP_AFFINE
