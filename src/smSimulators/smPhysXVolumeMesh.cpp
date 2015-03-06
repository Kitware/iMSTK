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

#include "smSimulators/smPhysXVolumeMesh.h"
#include "smRendering/smGLRenderer.h"

smPhysXVolumeMesh::smPhysXVolumeMesh()
{

    this->nbrTetra = 0;
    this->nbrNodes = 0;
    this->nbrLinks = 0;
    renderSurface = true;
    renderTetras = !renderSurface;
}

smPhysXVolumeMesh::~smPhysXVolumeMesh()
{

    if (neiTet != NULL)
    {
        for (smInt i = 0; this->nbrNodes; i++)
        {
            if (this->neiTet[i].idx != NULL)
            {
                delete [] this->neiTet[i].idx;
            }
        }

        delete [] neiTet;
    }
}

///loads the .tet and .obj file
smBool smPhysXVolumeMesh::loadTetFile(smChar *p_TetFileName, smChar *p_surfaceMesh)
{

    static const smInt MESH_STRING_LEN = 256;
    smChar s[MESH_STRING_LEN];
    smInt i0, i1, i2, i3;
    smVec3f v;

    FILE *f = fopen(p_TetFileName, "r");

    if (!f)
    {
        return false;
    }

    std::vector<smVec3f> tempNodes;
    std::vector<smInt> tempIndices;
    smPhysXLink link;

    while (!feof(f))
    {
        if (fgets(s, MESH_STRING_LEN, f) == NULL)
        {
            break;
        }

        if (strncmp(s, "v ", 2) == 0)   // vertex
        {
            sscanf(s, "v %f %f %f", &v.x, &v.y, &v.z);
            tempNodes.push_back(v);
        }
        else if (strncmp(s, "t ", 2) == 0)      // tetra
        {
            sscanf(s, "t %i %i %i %i", &i0, &i1, &i2, &i3);
            tempIndices.push_back(i0);
            tempIndices.push_back(i1);
            tempIndices.push_back(i2);
            tempIndices.push_back(i3);
        }
        else if (strncmp(s, "l ", 2) == 0)      // link
        {
            sscanf(s, "l %i %f %f %f", &link.tetraIndex, &link.baryCetricDistance[0], &link.baryCetricDistance[1], &link.baryCetricDistance[2]);
            link.baryCetricDistance[3] = 1 - (link.baryCetricDistance[0] + link.baryCetricDistance[1] + link.baryCetricDistance[2]);
            links.push_back(link);
        }
    }

    this->nbrTetra = tempIndices.size() / 4;
    this->nbrNodes = tempNodes.size();
    this->nbrLinks = links.size();

    this->tetra = new smTetrahedra[this->nbrTetra];
    this->nodes = new smVec3f[this->nbrNodes];

    for (smInt i = 0; i < this->nbrNodes; i++)
    {
        this->nodes[i] = tempNodes[i];
    }

    for (smInt i = 0; i < this->nbrTetra; i++)
    {
        this->tetra[i].vert[0] = tempIndices[4 * i];
        this->tetra[i].vert[1] = tempIndices[4 * i + 1];
        this->tetra[i].vert[2] = tempIndices[4 * i + 2];
        this->tetra[i].vert[3] = tempIndices[4 * i + 3];
    }

    surfaceMesh = new smSurfaceMesh(SMMESH_DEFORMABLE, NULL);
    surfaceMesh->loadMesh(p_surfaceMesh, SM_FILETYPE_OBJ);

    this->drawTet = new smBool[this->nbrTetra];

    for (smInt i = 0; i < this->nbrTetra; i++)
    {
        this->drawTet[i] = true;
    }

    return true;
}

void smPhysXVolumeMesh::updateSurfaceVertices()
{
    smVec3f t[4];

    for (smInt i = 0; i < this->nbrLinks; i++)
    {
        t[0] = this->nodes[this->tetra[links[i].tetraIndex].vert[0]];
        t[1] = this->nodes[this->tetra[links[i].tetraIndex].vert[1]];
        t[2] = this->nodes[this->tetra[links[i].tetraIndex].vert[2]];
        t[3] = this->nodes[this->tetra[links[i].tetraIndex].vert[3]];
        surfaceMesh->vertices[i] =
            t[0] * links[i].baryCetricDistance[0]
            + t[1] * links[i].baryCetricDistance[1]
            + t[2] * links[i].baryCetricDistance[2]
            + t[3] * links[i].baryCetricDistance[3];
    }
}

void smPhysXVolumeMesh::findNeighborTetrasOfNode()
{
    smInt i, j, k, n, tempIdx[1000];

    this->neiTet = new smNeighborTetrasOfNode[this->nbrNodes];

    for (i = 0 ; i < this->nbrNodes; i++)
    {
        n = 0;

        for (j = 0 ; j < this->nbrTetra; j++)
        {
            if (i == this->tetra[j].vert[0] ||
                    i == this->tetra[j].vert[1] ||
                    i == this->tetra[j].vert[2] ||
                    i == this->tetra[j].vert[3])
            {
                tempIdx[n] = j;
                n++;
            }
        }

        this->neiTet[i].nbrNeiTet = n;
        this->neiTet[i].idx = new smInt[this->neiTet[i].nbrNeiTet];

        for (j = 0; j < this->neiTet[i].nbrNeiTet; j++)
        {
            this->neiTet[i].idx[j] = tempIdx[j];
        }
    }
}

void smPhysXVolumeMesh::draw(smDrawParam p_params)
{

    if (renderSurface)
    {
        smGLRenderer::drawSurfaceMeshTriangles(this->surfaceMesh, &p_params.caller->renderDetail, p_params);
    }

    if (renderTetras)
    {
        glBegin(GL_TRIANGLES);

        for (smInt i = 0; i < this->nbrTetra; i++)
        {
            if (this->drawTet[i])
            {
                smVec3f p0(nodes[tetra[i].vert[0]]);
                smVec3f p1(nodes[tetra[i].vert[1]]);
                smVec3f p2(nodes[tetra[i].vert[2]]);
                smVec3f p3(nodes[tetra[i].vert[3]]);

                smVec3f normal, center, v[4];
                float scale = 0.9;

                center = p0;
                v[0] = p0;
                center += p1;
                v[1] = p1;
                center += p2;
                v[2] = p2;
                center += p3;
                v[3] = p3;
                center = center * 0.25f;
                v[0] = center + (v[0] - center) * scale;
                v[1] = center + (v[1] - center) * scale;
                v[2] = center + (v[2] - center) * scale;
                v[3] = center + (v[3] - center) * scale;
                smInt sides[4][3] = {{2, 1, 0}, {0, 1, 3}, {1, 2, 3}, {2, 0, 3}};

                for (smInt side = 0; side < 4; side ++)
                {
                    smVec3f &v0 = v[sides[side][0]];
                    smVec3f &v1 = v[sides[side][1]];
                    smVec3f &v2 = v[sides[side][2]];
                    smVec3f a = v1 - v0;
                    smVec3f b = v2 - v0;
                    normal = a.cross(b);
                    normal.normalize();
                    glNormal3fv((GLfloat*)&normal);
                    glVertex3fv((GLfloat*) &v0);
                    glVertex3fv((GLfloat*) &v1);
                    glVertex3fv((GLfloat*) &v2);
                }
            }
        }

        glEnd();
    }
}

void smPhysXVolumeMesh::createEdgeofTetras()
{

    smInt edgeNodes[6][2] = {{0, 1}, {0, 2}, {0, 3}, {1, 2}, {1, 3}, {2, 3}};
    tetraEdges.resize(nbrTetra);
    smEdge edge;
    smEdge existingEdge;
    smBool edgeExist;
    smUInt swap;

    for (smInt i = 0; i < nbrTetra; i++)
    {
        for (smInt j = 0; j < 6; j++)
        {
            edgeExist = false;
            edge.vert[0] = this->tetra[i].vert[edgeNodes[j][0]];
            edge.vert[1] = this->tetra[i].vert[edgeNodes[j][1]];

            if (edge.vert[1] < edge.vert[0])
            {
                swap = edge.vert[1];
                edge.vert[1] = edge.vert[0];
                edge.vert[0] = swap;
            }

            for (smInt m = 0; m < tetraEdges[edge.vert[0]].size(); m++)
            {
                existingEdge = tetraEdges[edge.vert[0]][m];

                if (existingEdge.vert[0] == edge.vert[0] && existingEdge.vert[1] == edge.vert[1])
                {
                    edgeExist = true;
                    break;
                }
            }

            if (!edgeExist)
            {
                tetraEdges[edge.vert[0]].push_back(edge);
            }
        }
    }
}
