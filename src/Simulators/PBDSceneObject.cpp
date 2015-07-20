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

#include "PBDSceneObject.h"

void smPBDSurfaceSceneObject::findFixedMassWrtSphere(core::Vec3d p_center, float p_radius)
{
    float dist = 0;

    for (int i = 0; i < mesh->nbrVertices; i++)
    {

        dist = (p_center - mesh->vertices[i]).norm();

        if (dist < p_radius)
        {
            fixedMass[i] = true;
        }
    }
}
smPBDSceneObject::smPBDSceneObject( std::shared_ptr<ErrorLog>/*p_log*/ )
{
    type = core::ClassType::PbdSceneObject;
}

std::shared_ptr<SceneObject> smPBDSceneObject::clone()
{
    return safeDownCast<SceneObject>();
}

void smPBDSceneObject::serialize( void */*p_memoryBlock*/ )
{

}

void smPBDSceneObject::unSerialize( void */*p_memoryBlock*/ )
{

}

smPBDSurfaceSceneObject::smPBDSurfaceSceneObject( std::shared_ptr<ErrorLog> p_log )
{
    type = core::ClassType::PbdSurfaceSceneObject;
    mesh = new SurfaceMesh( BaseMesh::MeshType::Deformable, p_log );
}
std::shared_ptr<SceneObject> smPBDSurfaceSceneObject::clone()
{
    return safeDownCast<SceneObject>();
}
void smPBDSurfaceSceneObject::serialize( void */*p_memoryBlock*/ )
{
}
void smPBDSurfaceSceneObject::unSerialize( void */*p_memoryBlock*/ )
{

}
void smPBDSurfaceSceneObject::initMeshStructure()
{
    P = 0;
    V = 0;
    exF = 0;
    L0 = 0;
    fixedMass = 0;
    listFixedMass = 0;
    paraK = 0.9;
    dT = 0.1;
    Damp = 0.0;
}
void smPBDSurfaceSceneObject::InitSurfaceObject()
{
    //surface mesh
    nbrMass = mesh->nbrVertices;

    P = new core::Vec3d[nbrMass];
    V = new core::Vec3d[nbrMass];
    exF = new core::Vec3d[nbrMass];
    fixedMass = new bool[nbrMass];

    for ( int i = 0; i < nbrMass; i++ )
    {
        fixedMass[i] = false;
    }

    for ( int i = 0; i < nbrMass; i++ )
    {
        P[i] = mesh->vertices[i];
    }

    nbrSpr = mesh->edges.size();
    L0 = new float[nbrSpr];

    for ( int i = 0; i < nbrSpr; i++ )
    {
        L0[i] = ( mesh->vertices[mesh->edges[i].vert[0]] - mesh->vertices[mesh->edges[i].vert[1]] ).norm();
    }

    mesh->allocateAABBTris();
}
smPBDSurfaceSceneObject::~smPBDSurfaceSceneObject()
{
    if ( P != 0 )
    {
        delete [] P;
    }

    if ( V != 0 )
    {
        delete [] V;
    }

    if ( exF != 0 )
    {
        delete [] exF;
    }

    if ( L0 != 0 )
    {
        delete [] L0;
    }

    if ( fixedMass != 0 )
    {
        delete [] fixedMass;
    }
}
void smPBDSurfaceSceneObject::findFixedCorners()
{

    nbrFixedMass = 2;
    listFixedMass = new int[nbrFixedMass];
    core::Vec3d corner[2];
    int i, j;
    float minmin, dist;
    corner[0] = mesh->aabb.aabbMax;
    corner[1] = mesh->aabb.aabbMin;
    listFixedMass[0] = -1;
    listFixedMass[1] = -1;

    for ( i = 0; i < nbrFixedMass; i++ )
    {
        minmin = std::numeric_limits<float>::max();

        for ( j = 0; j < mesh->nbrVertices; j++ )
        {
            dist = ( corner[i] - mesh->vertices[j] ).norm();

            if ( dist < minmin )
            {
                minmin = dist;
                listFixedMass[i] = j;
            }
        }
    }

    for ( i = 0; i < nbrFixedMass; i++ )
    {
        fixedMass[listFixedMass[i]] = true;
    }

    if ( listFixedMass != 0 )
    {
        delete [] listFixedMass;
    }
}
