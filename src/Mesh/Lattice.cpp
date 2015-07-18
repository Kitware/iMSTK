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

#include "Lattice.h"
#include "Core/Factory.h"

smLattice::smLattice()
{
    this->cells = NULL;
    this->totalCells = 0;
    this->xStep = 0;
    this->yStep = 0;
    this->zStep = 0;
    this->xSeperation = 0;
    this->ySeperation = 0;
    this->zSeperation = 0;
    this->setRenderDelegate(
      smFactory<smRenderDelegate>::createSubclass(
        "RenderDelegate", "LatticeRenderDelegate"));
}
float smLattice::getXStep()
{
    return xStep;
}
float smLattice::getYStep()
{
    return yStep;
}
float smLattice::getZStep()
{
    return zStep;
}
smVec3d smLattice::getLatticeCenter()
{
    return latticeCenter;
}
smVec3d smLattice::getLeftMinCorner()
{
    return cells[0].cellLeftCorner;
}
smVec3d smLattice::getRightMaxCorner()
{
    return cells[totalCells - 1].cellRightCorner;
}
smLattice::~smLattice()
{
    delete[] cells;
    delete[] aabb;
}
smLatticeReturnType smLattice::init( smVec3d p_leftCorner, smVec3d p_rightCorner, int p_xSeperation, int p_ySeperation, int p_zSeperation )
{

    smInt x, y, z;
    smInt index;

    xSeperation = p_xSeperation;
    ySeperation = p_ySeperation;
    zSeperation = p_zSeperation;

    boundingBoxInit();
    cells = new smCell[xSeperation * ySeperation * zSeperation];
    zStep = ( p_rightCorner[2] - p_leftCorner[2] ) / zSeperation;
    yStep = ( p_rightCorner[1] - p_leftCorner[1] ) / ySeperation;
    xStep = ( p_rightCorner[0] - p_leftCorner[0] ) / xSeperation;
    smInt counter = 0;

    for ( y = 0; y < ySeperation; y++ )
        for ( z = 0; z < zSeperation; z++ )
            for ( x = 0; x < xSeperation; x++ )
            {
                index = x + z * xSeperation + y * xSeperation * zSeperation;

                if ( (x < 0 || y < 0) | (z < 0 || x >= xSeperation || y >= ySeperation || z >= zSeperation) )
                {
                    printf( "Error index is out of bounds in createllatice function" );
                    return SIMMEDTK_LATTICE_INVALIDBOUNDS;
                }

                cells[index].id = index;
                cells[index].cellLeftCorner[0] = p_leftCorner[0] + x * xStep;
                cells[index].cellLeftCorner[1] = p_leftCorner[1] + y * yStep;
                cells[index].cellLeftCorner[2] = p_leftCorner[2] + z * zStep;

                cells[index].cellRightCorner[0] = cells[index].cellLeftCorner[0] + xStep;
                cells[index].cellRightCorner[1] = cells[index].cellLeftCorner[1] + yStep;
                cells[index].cellRightCorner[2] = cells[index].cellLeftCorner[2] + zStep;


                cells[index].cellCenter[0] = ( cells[index].cellLeftCorner[0] + cells[index].cellRightCorner[0] ) / 2;
                cells[index].cellCenter[1] = ( cells[index].cellLeftCorner[1] + cells[index].cellRightCorner[1] ) / 2;
                cells[index].cellCenter[2] = ( cells[index].cellLeftCorner[2] + cells[index].cellRightCorner[2] ) / 2;
                cells[index].isActive = false;
                cells[index].lastPrimitiveIndex = 0;

                for ( smInt j = 0; j < SIMMEDTK_SPATIALGRID_MAXPRIMITIVES; j++ )
                {
                    cells[index].cellPrimitives[j].index = 0;
                }

                counter++;
            }

    this->totalCells = counter;
    this->latticeCenter[0] = ( p_leftCorner[0] + p_rightCorner[0] ) / 2.0;
    this->latticeCenter[1] = ( p_leftCorner[1] + p_rightCorner[1] ) / 2.0;
    this->latticeCenter[2] = ( p_leftCorner[2] + p_rightCorner[2] ) / 2.0;

    return SIMMEDTK_LATTICE_OK;
}
void smLattice::indexReset()
{

    int traverseIndex = 0;

    for ( int y = 0; y < ySeperation; y++ )
        for ( int z = 0; z < zSeperation; z++ )
            for ( int x = 0; x < xSeperation; x++ )
            {
                traverseIndex = x + z * xSeperation + y * xSeperation * zSeperation;
                cells[traverseIndex].lastPrimitiveIndex = 0;
            }
}
void smLattice::isCellEmpty( int /*p_cellIndex*/ )
{
}
void smLattice::linkPrimitivetoCell( int p_primitiveIndex )
{

    smInt minX;
    smInt minY;
    smInt minZ;
    smInt maxX;
    smInt maxY;
    smInt maxZ;
    smInt index;
    smVec3d leftCorner = getLeftMinCorner();

    minX = ( aabb[p_primitiveIndex].aabbMin[0] - leftCorner[0] ) / xStep;
    minY = ( aabb[p_primitiveIndex].aabbMin[1] - leftCorner[1] ) / yStep;
    minZ = ( aabb[p_primitiveIndex].aabbMin[2] - leftCorner[2] ) / zStep;

    maxX = ( aabb[p_primitiveIndex].aabbMax[0] - leftCorner[0] ) / xStep;
    maxY = ( aabb[p_primitiveIndex].aabbMax[1] - leftCorner[1] ) / yStep;
    maxZ = ( aabb[p_primitiveIndex].aabbMax[2] - leftCorner[2] ) / zStep;

    for ( smInt yIndex = minY; yIndex <= maxY; yIndex++ )
        for ( smInt xIndex = minX; xIndex <= maxX; xIndex++ )
            for ( smInt zIndex = minZ; zIndex <= maxZ; zIndex++ )
            {
                index = xIndex + zIndex * xSeperation + yIndex * xSeperation * zSeperation;

                if ( ((xIndex < 0 || yIndex < 0)) || ((zIndex < 0 || xIndex >= xSeperation || yIndex >= ySeperation || zIndex >= zSeperation)) )
                {
                    continue;
                }

                if ( cells[index].lastPrimitiveIndex >= SIMMEDTK_SPATIALGRID_MAXPRIMITIVES )
                {
                    return;
                }

                cells[index].cellPrimitives[cells[index].lastPrimitiveIndex].index = p_primitiveIndex;
                cells[index].lastPrimitiveIndex++;
            }
}
void smLattice::updateBounds( std::shared_ptr<smSurfaceMesh> p_mesh, int p_index )
{

    //min
    aabb[p_index].aabbMin[0] =  SIMMEDTK_MIN( p_mesh->vertices[p_mesh->triangles[p_index].vert[0]][0],
                               p_mesh->vertices[p_mesh->triangles[p_index].vert[1]][0] );
    aabb[p_index].aabbMin[0] = SIMMEDTK_MIN( aabb[p_index].aabbMin[0],
                                            p_mesh->vertices[p_mesh->triangles[p_index].vert[2]][0] );

    aabb[p_index].aabbMin[1] =  SIMMEDTK_MIN( p_mesh->vertices[p_mesh->triangles[p_index].vert[0]][1],
                               p_mesh->vertices[p_mesh->triangles[p_index].vert[1]][1] );
    aabb[p_index].aabbMin[1] = SIMMEDTK_MIN( aabb[p_index].aabbMin[1],
                                            p_mesh->vertices[p_mesh->triangles[p_index].vert[2]][1] );

    aabb[p_index].aabbMin[2] =  SIMMEDTK_MIN( p_mesh->vertices[p_mesh->triangles[p_index].vert[0]][2],
                               p_mesh->vertices[p_mesh->triangles[p_index].vert[1]][2] );
    aabb[p_index].aabbMin[2] = SIMMEDTK_MIN( aabb[p_index].aabbMin[2],
                                            p_mesh->vertices[p_mesh->triangles[p_index].vert[2]][2] );

    //max
    aabb[p_index].aabbMax[0] =  SIMMEDTK_MAX( p_mesh->vertices[p_mesh->triangles[p_index].vert[0]][0],
                               p_mesh->vertices[p_mesh->triangles[p_index].vert[1]][0] );
    aabb[p_index].aabbMax[0] = SIMMEDTK_MAX( aabb[p_index].aabbMax[0],
                                            p_mesh->vertices[p_mesh->triangles[p_index].vert[2]][0] );

    aabb[p_index].aabbMax[1] =  SIMMEDTK_MAX( p_mesh->vertices[p_mesh->triangles[p_index].vert[0]][1],
                               p_mesh->vertices[p_mesh->triangles[p_index].vert[1]][1] );
    aabb[p_index].aabbMax[1] = SIMMEDTK_MAX( aabb[p_index].aabbMax[1],
                                            p_mesh->vertices[p_mesh->triangles[p_index].vert[2]][1] );

    aabb[p_index].aabbMax[2] =  SIMMEDTK_MAX( p_mesh->vertices[p_mesh->triangles[p_index].vert[0]][2],
                               p_mesh->vertices[p_mesh->triangles[p_index].vert[1]][2] );
    aabb[p_index].aabbMax[2] = SIMMEDTK_MAX( aabb[p_index].aabbMax[2],
                                            p_mesh->vertices[p_mesh->triangles[p_index].vert[2]][2] );
}
void smLattice::updateBounds()
{
    for ( smInt i = 0; i < mesh->nbrTriangles; i++ )
    {
        updateBounds( mesh, i );
    }
}
void smLattice::linkPrims()
{
    for ( smInt i = 0; i < mesh->nbrTriangles; i++ )
    {
        linkPrimitivetoCell( i );
    }
}
void smLattice::addObject( smSceneObject *obj )
{
    smClassType objectType;
    linkedObject = obj->getObjectUnifiedID();
    objectType = obj->getType();

    switch ( objectType )
    {
        case SIMMEDTK_SMSTATICSCENEOBJECT:
        {
            auto staticSceneObject = static_cast<smStaticSceneObject*>(obj);
            auto model = staticSceneObject->getModel();
            if(nullptr == model)
            {
                break;
            }
            std::shared_ptr<smMesh> mesh = model->getMesh();
            break;
        }
        default:
            std::cerr << "Unknown class type." << std::endl;
    }
}
