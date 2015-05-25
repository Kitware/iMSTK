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

#include "smMesh/smVolumeMesh.h"


/// \brief constructor
smVolumeMesh::smVolumeMesh(const smMeshType &p_meshtype, std::shared_ptr<smErrorLog> log)
{

    this->log_VM = log;
    meshType = p_meshtype;

}

/// \brief loads the specified volume mesh
smBool smVolumeMesh::loadMesh(const smString& fileName, const smMeshFileType &fileType)
{

    smBool ret;

    if (fileType == SM_FILETYPE_VOLUME)
    {
        ret = LoadTetra(fileName);

        if (ret == 0)
            if (log_VM != nullptr)
            {
                log_VM->addError("Error: Mesh file NOT FOUND");
            }
    }
    else
    {
        if (log_VM != nullptr)
        {
            log_VM->addError("Error: Mesh file type unidentified");
        }

        ret = 0;
    }

    return ret;
}

/// \brief
void smVolumeMesh::translateVolumeMesh(const smVec3d &p_offset)
{

    this->translate(p_offset);

    for (smInt i = 0; i < this->nbrNodes; i++)
    {
        nodes[i] += p_offset;
    }
}

/// \brief
void smVolumeMesh::scaleVolumeMesh(const smVec3d &p_offset)
{

    scale(p_offset);

    for (smInt i = 0; i < this->nbrNodes; i++)
    {
        nodes[i] = nodes[i].cwiseProduct(p_offset);
    }
}

/// \brief
void smVolumeMesh::rotVolumeMesh(const smMatrix33d &p_rot)
{

    rotate(p_rot);

    for (smInt i = 0; i < nbrNodes; i++)
    {
        nodes[i] = p_rot * nodes[i];
    }
}

/// \brief loads the tetra mesh from abacus
///Extensions to support other formats will come soon...
smBool smVolumeMesh::LoadTetra(const smString& fileName)
{

    smFloat number;
    smFloat numnodes;
    smChar comma;
    smChar stri[19];
    smInt i;

    FILE *fp = fopen(fileName.c_str(), "rb");

    if (!fp)
    {
        return false;
    }

    fscanf(fp, "%f%c\n", &numnodes, &comma);
    nbrNodes = numnodes;
    nodes.reserve(nbrNodes);
    fixed.reserve(nbrNodes);

    std::vector<smFloat> nodeNumber(nbrNodes);

    for (i = 0; i < nbrNodes; i++)
    {
        fixed.push_back(false);
    }


    for (i = 0; i < nbrNodes; i++)
    {
        nodes.emplace_back(smVec3d());
        smVec3d &node = nodes.back();
        fscanf(fp, "%f", &number);
        nodeNumber[i] = number;
        fscanf(fp, "%c", &comma);
        fscanf(fp, "%f", &number);
        node[0] = number;
        fscanf(fp, "%c", &comma);
        fscanf(fp, "%f", &number);
        node[1] = number;
        fscanf(fp, "%c", &comma);
        fscanf(fp, "%f", &number);
        node[2] = number;
        fscanf(fp, "\n");
    }

    fscanf(fp, "%s\n", stri);
    nbrTetra = 563;
    tetra.reserve(nbrTetra);

    for (i = 0; i < nbrTetra; i++)
    {
        tetra.emplace_back(smTetrahedra());
        smTetrahedra &tetrahedra = tetra.back();
        fscanf(fp, "%f", &number);
        fscanf(fp, "%c", &comma);
        fscanf(fp, "%f", &number);
        tetrahedra.vert[0] = number;
        fscanf(fp, "%c", &comma);
        fscanf(fp, "%f", &number);
        tetrahedra.vert[1] = number;
        fscanf(fp, "%c", &comma);
        fscanf(fp, "%f", &number);
        tetrahedra.vert[2] = number;
        fscanf(fp, "%c\n", &comma);
        fscanf(fp, "%f", &number);
        tetrahedra.vert[3] = number;
        fscanf(fp, "\n");
    }

    fclose(fp);
    return true;
}

/// \brief loads the surface vertices and triangles
smBool smVolumeMesh::getSurface(const smString& fileName)
{

    smFloat number;
    smInt j, i;
    smChar comma;
    smInt count;
    nbrTriangles = 777 - 564 + 1; //790....read here automatically

    initTriangleArrays(nbrTriangles);

    FILE *fp = fopen(fileName.c_str(), "rb");

    if (!fp)
    {
        return false;
    }

    smChar stri[19];
    fscanf(fp, "%s\n", stri);

    std::vector<smTriangle> triangles;
    for (i = 0; i < nbrTriangles; i++)
    {
        fscanf(fp, "%f", &number);
        fscanf(fp, "%c", &comma);
        fscanf(fp, "%f", &number);
        triangles[i].vert[0] = number;
        fscanf(fp, "%c", &comma);
        fscanf(fp, "%f", &number);
        triangles[i].vert[1] = number;
        fscanf(fp, "%c", &comma);
        fscanf(fp, "%f", &number);
        triangles[i].vert[2] = number;
        fscanf(fp, "\n");
    }

    fclose(fp);

    std::vector<bool> onSurface(nbrNodes);

    for (j = 0; j < nbrNodes; j++)
    {
        onSurface[j] = false;
    }

    for (j = 0; j < nbrTriangles; j++)
    {
        onSurface[triangles[j].vert[0] - 1] = onSurface[triangles[j].vert[1] - 1]
                = onSurface[triangles[j].vert[2] - 1] = true;
    }

    count = 0;
    std::vector<smInt> temp(nbrNodes);

    for (i = 0; i < nbrNodes; i++)
    {
        if (onSurface[i])
        {
            temp[i] = count;
            count++;
        }
        else
        {
            temp[i] = -1;
        }
    }

    initVertexArrays(count);

    for (i = 0; i < nbrTriangles; i++)
    {
        triangles[i].vert[0] = temp[triangles[i].vert[0] - 1];
        triangles[i].vert[1] = temp[triangles[i].vert[1] - 1];
        triangles[i].vert[2] = temp[triangles[i].vert[2] - 1];
    }

    surfaceNodeIndex.resize(nbrVertices);

    count = 0;

    for (i = 0; i < nbrNodes; i++)
    {
        if (onSurface[i])
        {
            surfaceNodeIndex[count] = i;
            count++;
        }
    }

    //copy the co-ordinates of the surface vertices
    initSurface();

    return true;

}

/// \brief loads the tetra mesh from abacus
///Extensions to support other formats will come soon...
smBool smVolumeMesh::readBC(const smString& fileName)
{
    smInt number;
    smChar comma;
    smInt i;

    FILE *fp = fopen(fileName.c_str(), "rb");

    if (!fp)
    {
        return false;
    }

    smInt numNodes = 311;

    for (i = 0; i < numNodes; i++)
    {
        fscanf(fp, "%d", &number);
        fixed[number - 1] = true;
        fscanf(fp, "%c", &comma);
    }

    fclose(fp);
    return true;
}

/// \brief copies the updated co-ordinates of the surface vertices only
void smVolumeMesh::copySurface()
{

    smInt i;

    for (i = 0; i < nbrVertices; i++)
    {
        vertices[i][0] = nodes[surfaceNodeIndex[i]][0];
        vertices[i][1] = nodes[surfaceNodeIndex[i]][1];
        vertices[i][2] = nodes[surfaceNodeIndex[i]][2];
    }

    updateTriangleNormals();
    updateVertexNormals();
}

/// \brief copies the updated co-ordinates of the surface vertices only
void smVolumeMesh::initSurface()
{
    smInt i;

    for (i = 0; i < nbrVertices; i++)
    {
        vertices[i][0] = nodes[surfaceNodeIndex[i]][0];
        vertices[i][1] = nodes[surfaceNodeIndex[i]][1];
        vertices[i][2] = nodes[surfaceNodeIndex[i]][2];

        origVerts[i][0] = nodes[surfaceNodeIndex[i]][0];
        origVerts[i][1] = nodes[surfaceNodeIndex[i]][1];
        origVerts[i][2] = nodes[surfaceNodeIndex[i]][2];
    }

    initVertexNeighbors();
    updateTriangleNormals();
    updateVertexNormals();
}

void smVolumeMesh::updateVolumeMeshDataFromVEGA_Format(const std::shared_ptr<const VolumetricMesh> vega3dMesh)
{
    smInt i, threeI;

    //copy the nodal co-ordinates
    for(i=0; i<this->nbrVertices ; i++)
    {
        threeI = 3*i;
        /*this->nodes[i][0] = (*nodes)[threeI];
        this->nodes[i][1] = (*nodes)[threeI+1];
        this->nodes[i][2] = (*nodes)[threeI+2];*/ 
    }
}

void smVolumeMesh::importVolumeMeshDataFromVEGA_Format(const std::shared_ptr<const VolumetricMesh> vega3dMesh, const bool preProcessingStage)
{
    smInt i, threeI, j;

    //temporary arrays
    smInt *numNodes, *numElements, *numVertsPerEle;
    smInt **elements;
    smDouble **nodes;

    vega3dMesh->exportMeshGeometry(numNodes, nodes, numElements, numVertsPerEle, elements);

    this->nbrTetra = *numElements;
	this->nbrNodes = *numNodes;

	this->tetra.clear();
    //copy the element connectivity information
    for(i=0; i<this->nbrTetra ; i++)
    {
        threeI = *numVertsPerEle * i;
        for(j=0; j<*numVertsPerEle ; j++)
        {
            tetra[i].vert[j] = (*elements)[threeI + j];
        }
    }

	this->nodes.resize(this->nbrNodes);
    //copy the nodal co-ordinates
    for(i=0; i<this->nbrVertices ; i++)
    {
        threeI = 3*i;
        this->nodes[i][0] = (*nodes)[threeI];
        this->nodes[i][1] = (*nodes)[threeI+1];
        this->nodes[i][2] = (*nodes)[threeI+2]; 
    }

    if(preProcessingStage)
    {
        // do something here!
    }

    //deallocate temporary arrays   
    delete [] numVertsPerEle;
    delete [] numElements;
    delete [] numNodes;

    for(i=0; i<this->nbrTetra ; i++)
    {
        delete [] elements[i];
    }
    delete [] elements;

	for (i = 0; i<this->nbrNodes; i++)
    {
        delete [] nodes[i];
    }
    delete [] nodes;

}

/// \brief destructor
smVolumeMesh::~smVolumeMesh()
{

}

smVolumeMesh::smVolumeMesh()
{
    nbrNodes = 0;
    nbrTetra = 0;
}
