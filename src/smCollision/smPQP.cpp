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

#include "smCollision/smPQP.h"


void smPQPSkeleton::setTransformation( smMatrix44d &p_trans )
{
    mat = p_trans;
    mR = p_trans;
    mT = p_trans;
}
smPQPSkeleton::smPQPSkeleton( smSurfaceMesh *p_mesh )
{
    mT.setValue( 0, 0, 0 );
    mR.setIdentity();

    mMesh = p_mesh;
    mPQPModel = new PQP_Model();
    mPQPModel->BeginModel();

    for ( int i = 0; i < mMesh->nbrTriangles; i++ )
        mPQPModel->AddTri( ( const PQP_REAL * )( &mMesh->vertices[mMesh->triangles[i].vert[0]] ),
                           ( const PQP_REAL * )&mMesh->vertices[mMesh->triangles[i].vert[1]],
                           ( const PQP_REAL * )&mMesh->vertices[mMesh->triangles[i].vert[2]], i );

    mPQPModel->EndModel();
}
smPQPCollision::smPQPCollision( smErrorLog *p_errorLog, int maxColResult ) : smObjectSimulator( p_errorLog )
{
    minCollisionDistance = 1.5; //default distance
    onlySpecificMeshId.ID = -1;
    pipePQP = new smPipe( "col_PQP", sizeof( smPQPResult ), maxColResult );
}
void smPQPCollision::draw(const smDrawParam &p_params )
{

    double matrixArray[16];
    smInt nbrSrcMeshes = mPQPSourceSkeletons.size();
    glPushMatrix();

    for ( smInt i = 0; i < nbrSrcMeshes; i++ )
    {
        mat.getMatrixForOpenGL( matrixArray );
        glMultMatrixd( matrixArray );
        smGLRenderer::drawSurfaceMeshTriangles( mPQPSourceSkeletons[i]->mMesh, &p_params.caller->renderDetail, p_params );
    }

    glPopMatrix();
}
void smPQPCollision::initDraw(const smDrawParam &p_param )
{
}
void smPQPCollision::syncBuffers()
{
}
void smPQPCollision::run()
{
}
void smPQPCollision::addDestinationMeshes( smSurfaceMesh *p_mesh )
{
    smPQPSkeleton *pqpSkeleton = new smPQPSkeleton( p_mesh );
    mPQPDestinationSkeletons.push_back( pqpSkeleton );
}
smPQPSkeleton *smPQPCollision::addSourceMeshes( smSurfaceMesh *p_mesh )
{
    smPQPSkeleton *pqpSkeleton = new smPQPSkeleton( p_mesh );
    mPQPSourceSkeletons.push_back( pqpSkeleton );
    return pqpSkeleton;
}
void smPQPCollision::checkCollision()
{

    smInt nbrDestMeshes = mPQPDestinationSkeletons.size();
    smInt nbrSrcMeshes = mPQPSourceSkeletons.size();
    smPQPSkeleton *src;
    smPQPSkeleton *dest;
    PQP_DistanceResult colRes;
    PQP_REAL rel_err = 0.00000;
    PQP_REAL abs_err = 0.00000;
    smPQPResult *collisionResult;
    smInt t = 0;
    minCollisionHappened = false;

    collisionResult = ( smPQPResult * )pipePQP->beginWrite();

    for ( smInt i = 0; i < nbrSrcMeshes; i++ )
    {
        src = mPQPSourceSkeletons[i];
        src->colRes.distance = 1e15;
        src->colSkel = NULL;

        for ( smInt j = 0; j < nbrDestMeshes; j++ )
        {
            dest = mPQPDestinationSkeletons[j];

            if ( onlySpecificMeshId.ID > 0 && !( onlySpecificMeshId == dest->mMesh->uniqueId ) )
            {
                continue;
            }

            PQP_Distance( &colRes, src->mR.e, ( float * )&src->mT, src->mPQPModel, dest->mR.e, ( float * )&dest->mT, dest->mPQPModel, rel_err, abs_err );

            if ( colRes.Distance() < src->colRes.distance )
            {

                src->colRes = colRes;
                src->colMeshId = mPQPDestinationSkeletons[j]->mMesh->uniqueId;
                src->colSkel = mPQPDestinationSkeletons[j];
            }

            if ( colRes.Distance() < minCollisionDistance )
            {
                minCollisionHappened = true;
                smVec3f p1;
                smVec3f p2;
                p1.setValue( colRes.p1[0], colRes.p1[1], colRes.p1[2] );
                p2.setValue( colRes.p2[0], colRes.p2[1], colRes.p2[2] );

                if ( t < pipePQP->getElements() )
                {
                    collisionResult[t].point1 = p1;
                    collisionResult[t].point2 = p2;
                    collisionResult[t].distance = colRes.Distance();
                    t++;
                }

                glPushMatrix();
                glMaterialfv( GL_FRONT_AND_BACK, GL_DIFFUSE, smColor::colorYellow.toGLColor() );
                glTranslatef( p1.x, p1.y, p1.z );
                glutSolidSphere( 2.0, 15, 15 );
                glPopMatrix();

                glPushMatrix();
                glMaterialfv( GL_FRONT_AND_BACK, GL_DIFFUSE, smColor::colorPink.toGLColor() );
                glTranslatef( p2.x, p2.y, p2.z );
                glutSolidSphere( 2.0, 15, 15 );
                glPopMatrix();
            }
        }
    }

    pipePQP->endWrite( t );
    pipePQP->acknowledgeValueListeners();
}
void smPQPCollision::smPQPCollision::handleEvent( smEvent *p_event )
{
    smHapticOutEventData *hapticEventData;
    smVec3d rightVector;

    switch ( p_event->eventType.eventTypeCode )
    {
        case SIMMEDTK_EVENTTYPE_HAPTICOUT:
            hapticEventData = ( smHapticOutEventData * )p_event->data;

            if ( hapticEventData->deviceId == 0 )
            {
            }

            break;
    }
}
