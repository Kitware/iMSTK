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

#ifndef SMPBDSCENEOBJECT_H
#define SMPBDSCENEOBJECT_H

// SimMedTK includes
#include "Core/Config.h"
#include "Core/SceneObject.h"
#include "Mesh/SurfaceMesh.h"
#include "Mesh/Mesh.h"

/// \brief Position based dynamics (PBD) object
class smPBDSceneObject: public smSceneObject
{
public:
    /// \brief constructor
    smPBDSceneObject(std::shared_ptr<smErrorLog> p_log = nullptr);

    /// \brief !!
    virtual std::shared_ptr<smSceneObject> clone();

    /// \brief !!
    virtual void serialize(void *p_memoryBlock);

    /// \brief !!
    virtual void unSerialize(void *p_memoryBlock);

    /// \brief find the masses that are fixed
    void findFixedMass();

public:
    smFloat dT; ///< size of time step
    smFloat paraK; ///< !!
    smFloat Damp; ///< damping values
    smInt nbrMass; ///< number of masses
    smInt **massIdx; ///< !!
    smStdVector3d P; ///< !! position
    smStdVector3d V; ///< !! velocity
    smStdVector3d exF; ///< external force
    smInt nbrSpr; ///< !! number of spheres
    smFloat *L0; ///< !! Initial length
    smBool *fixedMass; ///< true if masses are fixed
    smInt nbrFixedMass; ///< number of fixed masses
    smInt *listFixedMass; ///< list of IDs of masses that are fixed

    smVec3d ball_pos; ///< !! position of ball
    smVec3d ball_vel; ///< !! velocity of ball
    smVec3d ball_frc; ///< !!

    smFloat ball_mass; ///< !! mass of ball
    smFloat ball_rad; ///< !! radius of ball
};

/// \brief Position based dynamics (PBD) object for surface mesh (eg. cloth)
class smPBDSurfaceSceneObject: public smPBDSceneObject
{
public:

    /// \brief constructor
    smPBDSurfaceSceneObject(std::shared_ptr<smErrorLog> p_log = nullptr);

    /// \brief !!
    virtual std::shared_ptr<smSceneObject> clone() override;

    /// \brief !!
    virtual void serialize(void *p_memoryBlock) override;

    /// \brief !!
    virtual void unSerialize(void *p_memoryBlock) override;

    /// \brief initialize the mesh structure
    void initMeshStructure();

    /// \brief initialize surface PBD object
    void InitSurfaceObject();

    /// \brief destructor
    ~smPBDSurfaceSceneObject();

    /// \brief find the masses that will be fixed based on the spheres
    void findFixedMassWrtSphere(smVec3d p_center, smFloat pos);

    /// \brief find fixed corners
    void findFixedCorners();

public:
    friend class smPBDSurfaceRenderDetail;

    smSurfaceMesh *mesh; ///< surface mesh
    smInt nbrTri; ///< number of surface triangles
    smInt **triVertIdx; ///< !!
    smInt **sprInTris; ///< triangles that include a spring
};

#endif
