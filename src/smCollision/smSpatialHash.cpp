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

#include "smCollision/smSpatialHash.h"
#include "smCollision/smSurfaceTree.h"
#include "smCollision/smOctreeCell.h"
#include "smCore/smSDK.h"

void smSpatialHash::reset()
{
    cells.clearAll();
    cellLines.clearAll();
    cellsForTri2Line.clearAll();
    cellsForModelPoints.clearAll();
}

void smSpatialHash::addMesh(smMesh *p_mesh)
{
    meshes.push_back(p_mesh);
    p_mesh->allocateAABBTris();
}

void smSpatialHash::addMesh(smLineMesh *p_mesh)
{
    lineMeshes.push_back(p_mesh);
}

void smSpatialHash::removeMesh(smMesh *p_mesh)
{
    for (smInt i = 0; i < meshes.size(); i++)
        if (meshes[i]->uniqueId == p_mesh->uniqueId)
        {
            meshes.erase(meshes.begin()+i);
        }
}

smSpatialHash::~smSpatialHash()
{
    delete []   collidedPrims;
    delete []   collidedLineTris;
    delete []   collidedModelPoints;
}

smSpatialHash::smSpatialHash(smErrorLog *p_errorLog, smInt p_hashTableSize,
                             smFloat p_cellSizeX, smFloat p_cellSizeY,
                             smFloat p_cellSizeZ, smInt p_outOutputPrimSize):
    smObjectSimulator(p_errorLog),
    cells(p_hashTableSize),
    cellLines(p_hashTableSize),
    cellsForTri2Line(p_hashTableSize),
    cellsForModel(p_hashTableSize),
    cellsForModelPoints(p_hashTableSize)
{

    cellSizeX = p_cellSizeX;
    cellSizeY = p_cellSizeY;
    cellSizeZ = p_cellSizeZ;
    collidedPrims = new smCollidedTriangles[p_outOutputPrimSize];
    collidedLineTris = new smCollidedLineTris[p_outOutputPrimSize];
    collidedModelPoints = new smCollidedModelPoints[p_outOutputPrimSize];

    //initialize the  number of collisions
    nbrTriCollisions = 0;
    nbrLineTriCollisions = 0;
    nbrModelPointCollisions = 0;

    maxPrims = p_outOutputPrimSize;
	
	/////////////////////// FIXME: These are leacking. They are not deallocated in this class. ///////////
    pipe = new smPipe("col_hash_tri2line", sizeof(smCollidedLineTris), p_outOutputPrimSize);
    pipeTriangles = new smPipe("col_hash_tri2tri", sizeof(smCollidedTriangles), p_outOutputPrimSize);
    pipeModelPoints = new smPipe("col_hash_model2points", sizeof(smCollidedModelPoints), p_outOutputPrimSize);
	///////////////////////////////////////////////// FIXME //////////////////////////////////////////////
    enableDuplicateFilter = false;
}

void smSpatialHash::initCustom()
{

    smClassType type;
    smSceneObject *object;

    //do nothing for now
    for (smInt i = 0; i < meshes.size(); i++)
    {
        meshes[i]->allocateAABBTris();
    }
}

//void smSpatialHash::computeHash(smLineMesh *p_lineMesh,
void smSpatialHash::computeHash(smMesh *p_mesh, int *p_tris, int p_nbrTris)
{
    smCellTriangle triangle;
    smFloat xStartIndex, yStartIndex, zStartIndex;
    smFloat xEndIndex, yEndIndex, zEndIndex;

    for (int i = 0; i < p_nbrTris; i++)
    {

        triangle.primID = p_tris[i];
        xStartIndex = p_mesh->triAABBs[p_tris[i]].aabbMin.x / cellSizeX;
        yStartIndex = p_mesh->triAABBs[p_tris[i]].aabbMin.y / cellSizeY;
        zStartIndex = p_mesh->triAABBs[p_tris[i]].aabbMin.z / cellSizeZ;

        xEndIndex = p_mesh->triAABBs[p_tris[i]].aabbMax.x / cellSizeX;
        yEndIndex = p_mesh->triAABBs[p_tris[i]].aabbMax.y / cellSizeY;
        zEndIndex = p_mesh->triAABBs[p_tris[i]].aabbMax.z / cellSizeZ;

        for (smInt ix = xStartIndex; ix <= xEndIndex; ix++)
            for (smInt iy = yStartIndex; iy <= yEndIndex; iy++)
                for (smInt iz = zStartIndex; iz <= zEndIndex; iz++)
                {
                    cells.insert(triangle, HASH(cells.tableSize, ix, iy, iz));
                }
    }
}

inline void smSpatialHash::addTriangle(smMesh *p_mesh, smInt p_triangleId, smHash<smCellTriangle> &p_cells)
{

    smFloat xStartIndex, yStartIndex, zStartIndex;
    smFloat  xEndIndex, yEndIndex, zEndIndex;
    smCellTriangle  triangle;
    triangle.meshID = p_mesh->uniqueId;
    triangle.primID = p_triangleId;
    triangle.vert[0] = p_mesh->vertices[p_mesh->triangles[p_triangleId].vert[0]];
    triangle.vert[1] = p_mesh->vertices[p_mesh->triangles[p_triangleId].vert[1]];
    triangle.vert[2] = p_mesh->vertices[p_mesh->triangles[p_triangleId].vert[2]];

    xStartIndex = (smInt)(p_mesh->triAABBs[p_triangleId].aabbMin.x / cellSizeX);
    yStartIndex = (smInt)(p_mesh->triAABBs[p_triangleId].aabbMin.y / cellSizeY);
    zStartIndex = (smInt)(p_mesh->triAABBs[p_triangleId].aabbMin.z / cellSizeZ);

    xEndIndex = (smInt)(p_mesh->triAABBs[p_triangleId].aabbMax.x / cellSizeX);
    yEndIndex = (smInt)(p_mesh->triAABBs[p_triangleId].aabbMax.y / cellSizeY);
    zEndIndex = (smInt)(p_mesh->triAABBs[p_triangleId].aabbMax.z / cellSizeZ);

    for (smInt ix = xStartIndex; ix <= xEndIndex; ix++)
        for (smInt iy = yStartIndex; iy <= yEndIndex; iy++)
            for (smInt iz = zStartIndex; iz <= zEndIndex; iz++)
            {
                p_cells.checkAndInsert(triangle, HASH(cells.tableSize, ix, iy, iz));
            }
}

inline smBool smSpatialHash::findCandidateTris(smMesh *p_mesh, smMesh *p_mesh2)
{

    smAABB aabboverlap;

    if (smCollisionUtils::checkOverlapAABBAABB(p_mesh->aabb, p_mesh2->aabb, aabboverlap) == false)
    {
        return false;
    }

    for (int i = 0; i < p_mesh->nbrTriangles; i++)
    {
        addTriangle(p_mesh, i, cells);
    }

    for (int i = 0; i < p_mesh2->nbrTriangles; i++)
    {
        addTriangle(p_mesh2, i, cells);
    }

    return true;
}

inline void smSpatialHash::addLine(smLineMesh *p_mesh,
                                   smInt p_edgeId, smHash<smCellLine> &p_cells)
{

    smFloat xStartIndex, yStartIndex, zStartIndex;
    smFloat  xEndIndex, yEndIndex, zEndIndex;
    smCellLine  line;
    line.meshID = p_mesh->uniqueId;
    line.primID = p_edgeId;
    line.vert[0] = p_mesh->vertices[p_mesh->edges[p_edgeId].vert[0]];
    line.vert[1] = p_mesh->vertices[p_mesh->edges[p_edgeId].vert[1]];


    xStartIndex = (smInt)(p_mesh->edgeAABBs[p_edgeId].aabbMin.x / cellSizeX);
    yStartIndex = (smInt)(p_mesh->edgeAABBs[p_edgeId].aabbMin.y / cellSizeY);
    zStartIndex = (smInt)(p_mesh->edgeAABBs[p_edgeId].aabbMin.z / cellSizeZ);

    xEndIndex = (smInt)(p_mesh->edgeAABBs[p_edgeId].aabbMax.x / cellSizeX);
    yEndIndex = (smInt)(p_mesh->edgeAABBs[p_edgeId].aabbMax.y / cellSizeY);
    zEndIndex = (smInt)(p_mesh->edgeAABBs[p_edgeId].aabbMax.z / cellSizeZ);

    for (smInt ix = xStartIndex; ix <= xEndIndex; ix++)
        for (smInt iy = yStartIndex; iy <= yEndIndex; iy++)
            for (smInt iz = zStartIndex; iz <= zEndIndex; iz++)
            {
                p_cells.checkAndInsert(line, HASH(cells.tableSize, ix, iy, iz));
            }
}

inline smBool smSpatialHash::findCandidateTrisLines(smMesh *p_mesh, smLineMesh *p_mesh2)
{

    smAABB aabboverlap;

    if (smCollisionUtils::checkOverlapAABBAABB(p_mesh->aabb, p_mesh2->aabb, aabboverlap) == false)
    {
        return false;
    }

    for (int i = 0; i < p_mesh->nbrTriangles; i++)
    {
        if (smCollisionUtils::checkOverlapAABBAABB(aabboverlap, p_mesh->triAABBs[i]))
        {
            addTriangle(p_mesh, i, cellsForTri2Line);
        }
    }

    for (int i = 0; i < p_mesh2->nbrEdges; i++)
    {
        if (smCollisionUtils::checkOverlapAABBAABB(aabboverlap, p_mesh2->edgeAABBs[i]))
        {
            addLine(p_mesh2, i, cellLines);
        }
    }

    return true;
}

void smSpatialHash::computeCollisionTri2Tri()
{

    int t = 0;
    smHashIterator<smCellTriangle > iterator;
    smHashIterator<smCellTriangle > iterator1;

    smCellTriangle tri;
    smCellTriangle tri1;
    smCollidedTriangles *tristris;
    smVec3f proj1, proj2, inter1, inter2;
    smShort point1, point2;
    smInt coPlanar;

    tristris = (smCollidedTriangles*)pipeTriangles->beginWrite();

    while (cells.next(iterator))
    {
        while (cells.nextBucketItem(iterator, tri))
        {
            iterator1.clone(iterator);

            while (cells.nextBucketItem(iterator1, tri1))
            {
                if (tri.meshID == tri1.meshID || !(smSDK::getMesh(tri.meshID)->collisionGroup.isCollisionPermitted(smSDK::getMesh(tri1.meshID)->collisionGroup)))
                {
                    continue;
                }

                if (t < maxPrims && smCollisionUtils::tri2tri(tri.vert[0],
                        tri.vert[1], tri.vert[2], tri1.vert[0], tri1.vert[1], tri1.vert[2]
                        , coPlanar, inter1, inter2, point1, point2, proj1, proj2))
                {
                    t++;
                    collidedPrims[t].tri1 = tri;
                    collidedPrims[t].tri2 = tri1;
                    tristris[t].tri1 = tri;
                    tristris[t].tri2 = tri1;
                    tristris[t].proj1 = proj1;
                    tristris[t].proj2 = proj2;
                    tristris[t].point1 = point1;
                    tristris[t].point2 = point2;
                }
            }
        }
    }

    pipeTriangles->endWrite(t);
    pipeTriangles->acknowledgeValueListeners();
    nbrTriCollisions = t;
}

inline smInt compareLineTris(const void* p_element1, const void* p_element2)
{

    smCollidedLineTris* p_1 = (smCollidedLineTris*)p_element1;
    smCollidedLineTris* p_2 = (smCollidedLineTris*)p_element2;

    return (p_1->line.primID - p_2->line.primID);
}

inline void  smSpatialHash::filterLine2TrisResults()
{

    smCollidedLineTris *lineTris;
    smUInt currentLinePrimID;
    smUInt previousLinePrimID;
    smInt indexofPreviousPrim;
    smInt t = 0;
    smBool existed = false;

    lineTris = (smCollidedLineTris*)pipe->beginWrite();

    if (nbrLineTriCollisions <= 0)
    {
        pipe->endWrite(0);
        pipe->acknowledgeValueListeners();
        return;
    }

    if (enableDuplicateFilter)
    {

        qsort(collidedLineTris, nbrLineTriCollisions, sizeof(smCollidedLineTris), compareLineTris);
        indexofPreviousPrim = 0;
        previousLinePrimID = collidedLineTris[0].line.primID;

        for (smInt i = 0; i < nbrLineTriCollisions; i++)
        {

            currentLinePrimID = collidedLineTris[i].line.primID;

            if (previousLinePrimID != currentLinePrimID)
            {
                indexofPreviousPrim = i;
                previousLinePrimID = currentLinePrimID;
                lineTris[t] = collidedLineTris[i];
                t++;
            }
            else
            {
                existed = false;

                for (smInt j = indexofPreviousPrim; j < i; j++)
                {
                    if (collidedLineTris[j].tri.primID == collidedLineTris[i].tri.primID)
                    {
                        existed = true;
                    }
                }

                if (!existed)
                {
                    lineTris[t] = collidedLineTris[i];
                    t++;
                }
            }
        }

    }
    else
    {
        memcpy(lineTris, collidedLineTris, sizeof(smCollidedLineTris)*nbrLineTriCollisions);
        t = nbrLineTriCollisions;
    }

    pipe->endWrite(t);
    pipe->acknowledgeValueListeners();
}

///line to triangle collision
void  smSpatialHash::computeCollisionLine2Tri()
{

    smInt t = 0;
    smHashIterator<smCellLine > iteratorLine;
    smHashIterator<smCellTriangle > iteratorTri;
    smCollidedLineTris *lineTris;
    smCellLine line;
    smCellTriangle tri;
    smVec3<smFloat> intersection;

    while (cellLines.next(iteratorLine) && cellsForTri2Line.next(iteratorTri))
    {
        while (cellLines.nextBucketItem(iteratorLine, line))
        {

            iteratorTri.resetBucketIteration();

            while (cellsForTri2Line.nextBucketItem(iteratorTri, tri))
            {

                if (tri.meshID == line.meshID || !(smSDK::getMesh(tri.meshID)->collisionGroup.isCollisionPermitted(smSDK::getMesh(line.meshID)->collisionGroup)))
                {
                    continue;
                }

                if (t < maxPrims && smCollisionUtils::checkLineTri(line.vert[0], line.vert[1], tri.vert[0], tri.vert[1], tri.vert[2], intersection))
                {
                    collidedLineTris[nbrLineTriCollisions].line = line;
                    collidedLineTris[nbrLineTriCollisions].tri = tri;
                    collidedLineTris[nbrLineTriCollisions].intersection = intersection;
                    nbrLineTriCollisions++;
                }
            }
        }
    }

    filterLine2TrisResults();
}

void smSpatialHash::initDraw(smDrawParam p_param)
{

    smViewer *viewer;
    viewer = p_param.rendererObject;
    viewer->addText(QString("smhash"));
}

void smSpatialHash::draw(smDrawParam p_param)
{

    smViewer *viewer;
    QString fps("Collision FPS: %1 TimePerFrame: %2");
    fps = fps.arg(smDouble(this->FPS)).arg(smDouble(this->timerPerFrame));

    viewer = p_param.rendererObject;

    glDisable(GL_LIGHTING);
    glColor3fv(smColor::colorWhite.toGLColor());

    glBegin(GL_TRIANGLES);

    for (smInt i = 0; i < nbrTriCollisions; i++)
    {

        glVertex3fv((GLfloat*)&collidedPrims[i].tri1.vert[0]);
        glVertex3fv((GLfloat*)&collidedPrims[i].tri1.vert[1]);
        glVertex3fv((GLfloat*)&collidedPrims[i].tri1.vert[2]);

        glVertex3fv((GLfloat*)&collidedPrims[i].tri2.vert[0]);
        glVertex3fv((GLfloat*)&collidedPrims[i].tri2.vert[1]);
        glVertex3fv((GLfloat*)&collidedPrims[i].tri2.vert[2]);
    }

    glEnd();

    glLineWidth(10.0);

    glBegin(GL_LINES);

    for (smInt i = 0; i < nbrLineTriCollisions; i++)
    {
        glVertex3fv((GLfloat*)&collidedLineTris[i].line.vert[0]);
        glVertex3fv((GLfloat*)&collidedLineTris[i].line.vert[1]);
    }

    glEnd();

    glLineWidth(1.0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

inline void smSpatialHash::addOctreeCell(smSurfaceTree<smOctreeCell> *p_colModel, smHash<smCellModel> p_cells)
{

    smFloat xStartIndex, yStartIndex, zStartIndex;
    smFloat  xEndIndex, yEndIndex, zEndIndex;
    smCellModel cellModel;
    smAABB temp;

    smSurfaceTreeIterator<smOctreeCell> iter = p_colModel->getLevelIterator();
    cellModel.meshID = p_colModel->getAttachedMeshID();

    for (smInt i = iter.start(); i < iter.end(); i++)
    {
        if (iter[i].filled)
        {

            temp.aabbMin =  iter[i].getCube().leftMinCorner();
            temp.aabbMax =  iter[i].getCube().rightMaxCorner();
            xStartIndex = (smInt)(temp.aabbMin.x / cellSizeX);
            yStartIndex = (smInt)(temp.aabbMin.y / cellSizeY);
            zStartIndex = (smInt)(temp.aabbMin.z / cellSizeZ);
            xEndIndex = (smInt)(temp.aabbMax.x / cellSizeX);
            yEndIndex = (smInt)(temp.aabbMax.y / cellSizeY);
            zEndIndex = (smInt)(temp.aabbMax.z / cellSizeZ);
            cellModel.primID = i;
            cellModel.center = iter[i].getCube().center;
            cellModel.radius = iter[i].getCube().getCircumscribedSphere().radius;

            for (smInt ix = xStartIndex; ix <= xEndIndex; ix++)
                for (smInt iy = yStartIndex; iy <= yEndIndex; iy++)
                    for (smInt iz = zStartIndex; iz <= zEndIndex; iz++)
                    {
                        p_cells.checkAndInsert(cellModel, HASH(cells.tableSize, ix, iy, iz));
                    }

        }

    }
}

inline void smSpatialHash::addPoint(smMesh *p_mesh, smInt p_vertId, smHash<smCellPoint> p_cells)
{
    smFloat xStartIndex, yStartIndex, zStartIndex;
    smFloat  xEndIndex, yEndIndex, zEndIndex;
    smCellPoint cellPoint;
    cellPoint.meshID = p_mesh->uniqueId;
    cellPoint.primID = p_vertId;
    cellPoint.vert = p_mesh->vertices[p_vertId];

    xStartIndex = (smInt)(p_mesh->vertices[p_vertId].x / cellSizeX);
    yStartIndex = (smInt)(p_mesh->vertices[p_vertId].y / cellSizeY);
    zStartIndex = (smInt)(p_mesh->vertices[p_vertId].z / cellSizeZ);

    p_cells.checkAndInsert(cellPoint, HASH(cells.tableSize, xStartIndex, yStartIndex, zStartIndex));
}

void  smSpatialHash::findCandidatePoints(smMesh *p_mesh, smSurfaceTree<smOctreeCell> *p_colModel)
{
    smAABB tempAABB;
    tempAABB.aabbMin = p_colModel->root.getCube().leftMinCorner();
    tempAABB.aabbMax = p_colModel->root.getCube().rightMaxCorner();

    for (smInt i = 0; i < p_mesh->nbrVertices; i++)
    {
        if (smCollisionUtils::checkAABBPoint(tempAABB, p_mesh->vertices[i]))
        {
            addPoint(p_mesh, i, cellsForModelPoints);
        }
    }
}

void smSpatialHash::computeCollisionModel2Points()
{

    int t = 0;
    smFloat distanceFromCenter;
    smHashIterator<smCellModel > iteratorModel;
    smHashIterator<smCellPoint > iteratorPoint;
    smCellModel model;
    smCellPoint point;
    smCollidedModelPoints *collidedModelPointsPipe;

    collidedModelPointsPipe = (smCollidedModelPoints*)pipeModelPoints->beginWrite();

    while (cellsForModel.next(iteratorModel) && cellsForModelPoints.next(iteratorPoint))
    {
        while (cellsForModel.nextBucketItem(iteratorModel, model))
        {

            iteratorPoint.resetBucketIteration();

            while (cellsForModelPoints.nextBucketItem(iteratorPoint, point))
            {

                distanceFromCenter = model.center.distance(point.vert);

                if (t < maxPrims && distanceFromCenter < model.radius)
                {
                    collidedModelPoints[t].penetration = model.radius - distanceFromCenter;
                    collidedModelPoints[t].model = model;
                    collidedModelPoints[t].point = point;

                    collidedModelPointsPipe[t].penetration = model.radius - distanceFromCenter;
                    collidedModelPointsPipe[t].model = model;
                    collidedModelPointsPipe[t].point = point;
                    t++;
                }
            }
        }
    }

    pipeModelPoints->endWrite(t);
    pipeModelPoints->acknowledgeValueListeners();
    nbrModelPointCollisions = t;
}

void smSpatialHash::addCollisionModel(smSurfaceTree<smOctreeCell> *p_CollMode)
{

    colModel.push_back(p_CollMode);
}

void smSpatialHash::run()
{

    smTimer timer;
    timer.start();
    beginSim();

    for (smInt i = 0; i < colModel.size(); i++)
        for (smInt i = 0; i < meshes.size(); i++)
        {
            findCandidatePoints(meshes[i], colModel[i]);
            addOctreeCell(colModel[i], cellsForModel);
        }

    ///Triangle-Triangle collision
    for (smInt i = 0; i < meshes.size(); i++)
    {
        for (smInt j = i + 1; j < meshes.size(); j++)
        {
            if (meshes[i]->collisionGroup.isCollisionPermitted(meshes[j]->collisionGroup))
            {
                if (findCandidateTris(meshes[i], meshes[j]) == false)
                {
                    continue;
                }
            }
        }
    }

    ///Triangle-line Collision
    for (smInt i = 0; i < meshes.size(); i++)
        for (smInt j = 0; j < lineMeshes.size(); j++)
        {
            if (meshes[i]->collisionGroup.isCollisionPermitted(lineMeshes[j]->collisionGroup))
            {
                if (findCandidateTrisLines(meshes[i], lineMeshes[j]) == false)
                {
                    continue;
                }
            }
        }

    computeCollisionTri2Tri();
    computeCollisionLine2Tri();
    computeCollisionModel2Points();
    endSim();
}

void smSpatialHash::beginSim()
{

    smObjectSimulator::beginSim();
    //start the job
    nbrTriCollisions = 0;
    nbrLineTriCollisions = 0;
    nbrModelPointCollisions = 0;

    for (smInt i = 0; i < meshes.size(); i++)
    {
        meshes[i]->updateTriangleAABB();
    }

    for (smInt i = 0; i < lineMeshes.size(); i++)
    {
        meshes[i]->upadateAABB();
    }
}
void smSpatialHash::endSim()
{
    //end the job
    smObjectSimulator::endSim();
    reset();
}
void smSpatialHash::syncBuffers()
{
}
