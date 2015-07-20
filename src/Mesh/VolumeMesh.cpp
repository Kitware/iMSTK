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

#include "VolumeMesh.h"


/// \brief constructor
VolumeMesh::VolumeMesh(const MeshType &p_meshtype, std::shared_ptr<ErrorLog> log)
{

    this->log_VM = log;
    meshType = p_meshtype;

}

/// \brief loads the specified volume mesh
bool VolumeMesh::loadMesh(const std::string& fileName, const MeshFileType &fileType)
{

    bool ret;

    if (fileType == BaseMesh::MeshFileType::Volume)
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
void VolumeMesh::translateVolumeMesh(const core::Vec3d &p_offset)
{

    this->translate(p_offset);

    for (int i = 0; i < this->nbrNodes; i++)
    {
        nodes[i] += p_offset;
    }
}

/// \brief
void VolumeMesh::scaleVolumeMesh(const core::Vec3d &p_offset)
{

    scale(p_offset);

    for (int i = 0; i < this->nbrNodes; i++)
    {
        nodes[i] = nodes[i].cwiseProduct(p_offset);
    }
}

/// \brief
void VolumeMesh::rotVolumeMesh(const Matrix33d &p_rot)
{

    rotate(p_rot);

    for (int i = 0; i < nbrNodes; i++)
    {
        nodes[i] = p_rot * nodes[i];
    }
}

/// \brief loads the tetra mesh from abacus
///Extensions to support other formats will come soon...
bool VolumeMesh::LoadTetra(const std::string& fileName)
{

    float number;
    float numnodes;
    char comma;
    char stri[19];
    int i;

    FILE *fp = fopen(fileName.c_str(), "rb");

    if (!fp)
    {
        return false;
    }

    fscanf(fp, "%f%c\n", &numnodes, &comma);
    nbrNodes = numnodes;
    nodes.reserve(nbrNodes);
    fixed.reserve(nbrNodes);

    std::vector<float> nodeNumber(nbrNodes);

    for (i = 0; i < nbrNodes; i++)
    {
        fixed.push_back(false);
    }


    for (i = 0; i < nbrNodes; i++)
    {
        nodes.emplace_back(core::Vec3d());
        core::Vec3d &node = nodes.back();
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
        tetra.emplace_back(Tetrahedra());
        Tetrahedra &tetrahedra = tetra.back();
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
bool VolumeMesh::getSurface(const std::string& fileName)
{

    float number;
    int j, i;
    char comma;
    int count;
    nbrTriangles = 777 - 564 + 1; //790....read here automatically

    initTriangleArrays(nbrTriangles);

    FILE *fp = fopen(fileName.c_str(), "rb");

    if (!fp)
    {
        return false;
    }

    char stri[19];
    fscanf(fp, "%s\n", stri);

    std::vector<Triangle> triangles;
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
    std::vector<int> temp(nbrNodes);

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
bool VolumeMesh::readBC(const std::string& fileName)
{
    int number;
    char comma;
    int i;

    FILE *fp = fopen(fileName.c_str(), "rb");

    if (!fp)
    {
        return false;
    }

    int numNodes = 311;

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
void VolumeMesh::copySurface()
{

    int i;

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
void VolumeMesh::initSurface()
{
    int i;

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
// WIP
void VolumeMesh::updateVolumeMeshFromVegaFormat(const std::shared_ptr<const VolumetricMesh> /*vega3dMesh*/)
{
//     int i, threeI;

    //copy the nodal co-ordinates
//     for(i=0; i<this->nbrVertices ; i++)
//     {
//         threeI = 3*i;
        /*this->nodes[i][0] = (*nodes)[threeI];
        this->nodes[i][1] = (*nodes)[threeI+1];
        this->nodes[i][2] = (*nodes)[threeI+2];*/
//     }
}

void VolumeMesh::importVolumeMeshFromVegaFormat(const std::shared_ptr<const VolumetricMesh> vega3dMesh, const bool preProcessingStage)
{
    int i, threeI, j;

    //temporary arrays
    int numNodes(0);
    int numElements(0);
    int numVertsPerEle(0);
    int *elements;
    double *nodes;

    vega3dMesh->exportMeshGeometry(&numNodes, &nodes, &numElements, &numVertsPerEle, &elements);

    this->nbrTetra = numElements;
	this->nbrNodes = numNodes;

    this->tetra.resize(this->nbrTetra);
    //copy the element connectivity information
    for(i=0; i<this->nbrTetra ; i++)
    {
        threeI = numVertsPerEle * i;
        for(j=0; j<numVertsPerEle ; j++)
        {
            tetra[i].vert[j] = elements[threeI + j];
        }
    }

	this->nodes.resize(this->nbrNodes);
    //copy the nodal co-ordinates
    for(i=0; i<this->nbrVertices ; i++)
    {
        threeI = 3*i;
        this->nodes[i][0] = nodes[threeI];
        this->nodes[i][1] = nodes[threeI+1];
        this->nodes[i][2] = nodes[threeI+2];
    }

    //WIP no original position data element in volume mesh
    if(preProcessingStage)
    {
        // do something here!
    }

    //deallocate temporary arrays
    delete [] elements;
    delete [] nodes;

}

/// \brief destructor
VolumeMesh::~VolumeMesh()
{

}

VolumeMesh::VolumeMesh()
{
    nbrNodes = 0;
    nbrTetra = 0;
}
