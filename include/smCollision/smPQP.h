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

#ifndef SMPQP_H
#define SMPQP_H

#include "smCore/smConfig.h"
#include "smCollision/smCollisionConfig.h"
#include "smCore/smObjectSimulator.h"
#include "smCore/smErrorLog.h"
#include "smCore/smEventData.h"
#include "smMesh/smMesh.h"
#include "smCore/smDataStructures.h"
#include "smCore/smConfig.h"
#include "smCore/smCoreClass.h"
#include "PQP.h"
#include "smRendering/smGLRenderer.h"

class smPipe;

/// \brief holds the result of the PQP collision
struct smPQPResult
{
    smVec3d point1;
    smVec3d point2;
    smFloat distance;
};

/// \brief
class smPQPSkeleton: public smCoreClass
{

public:
    smMatrix33d mR; ///<
    smVec3d mT; ///<
    smMatrix44f mat; ///<

    PQP_Model  *mPQPModel; ///< PQP model
    smSurfaceMesh *mMesh; ///< surface mesh
    PQP_DistanceResult colRes; ///< PQP results
    std::shared_ptr<smUnifiedId> colMeshId; ///< stores the closest mesh id
    smPQPSkeleton *colSkel; ///< !!

    /// \brief !! set the transforms
    void setTransformation(smMatrix44d &p_trans);

    /// \brief initialize the PQP model using surface mesh
    smPQPSkeleton(smSurfaceMesh *p_mesh);

};

///PQP based collision detection
class smPQPCollision: public smObjectSimulator
{

public:

    smMatrix44d mat; ///< !!
    smVec3d pos; ///< !!
    smFloat minCollisionDistance; ///< The default value is 1.0
    smBool  minCollisionHappened; ///<
    std::shared_ptr<smUnifiedId>   onlySpecificMeshId; ///< collision check is done only with a specific mesh if this is given. This will have the mesh id;
    smPipe *pipePQP; ///< !!

    /// \brief constructor
    smPQPCollision(smErrorLog *p_errorLog = NULL, smInt maxColResult = 500);

    /// \brief !!
    void initCustom()
    {
    }

    /// \brief rendering the collision meshes
    void draw(const smDrawParam &p_params);

    /// \brief !!
    void initDraw(const smDrawParam &p_param);

    /// \brief !!
    void syncBuffers();

    virtual void run();

    std::vector<smPQPSkeleton *>mPQPSourceSkeletons; ///<
    std::vector<smPQPSkeleton *>mPQPDestinationSkeletons; ///<

    /// \brief !!
    void addDestinationMeshes(smSurfaceMesh *p_mesh);

    /// \brief !!
    smPQPSkeleton* addSourceMeshes(smSurfaceMesh *p_mesh);

    /// \brief check the collision
    void checkCollision();

    /// \brief !!
    void handleEvent(std::shared_ptr<smtk::Event::smEvent> event);
};

#endif
