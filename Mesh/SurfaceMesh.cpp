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

#include "Mesh/SurfaceMesh.h"

#include <assert.h>

//assimp includes
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#ifdef _WIN32
#define fileno _fileno
#include "io.h"
int Filelength(const char*, int id)
{
    return filelength(id);
}

#else

#include<sys/stat.h>

size_t Filelength(const char * filename, int)
{
    struct stat st;
    stat(filename, &st);
    return st.st_size;
}
#endif

/// \brief constructor
SurfaceMesh::SurfaceMesh(const MeshType &p_meshtype, std::shared_ptr<ErrorLog> log)
{

    this->log_SF = log;
    meshType = p_meshtype;
    meshFileType = BaseMesh::MeshFileType::None;
}

/// \brief destructor
SurfaceMesh::~SurfaceMesh()
{

}

/// \brief loads the mesh based on the file type and initializes the normals
bool SurfaceMesh::loadMesh(const std::string& fileName, const MeshFileType &fileType)
{

    bool ret = true;

    switch (fileType)
    {
    case BaseMesh::MeshFileType::ThreeDS:
    case BaseMesh::MeshFileType::Obj:
        meshFileType = fileType;
        ret = LoadMeshAssimp(fileName);
        break;

    default:
        if (log_SF != NULL)
        {
            log_SF->addError("Error: Mesh file TYPE UNIDENTIFIED");
        }

        ret = false;
    }

    assert(ret);

    if (ret == false)
    {
        if (log_SF != NULL)
        {
            log_SF->addError("Error: Mesh file NOT FOUND");
        }
    }

    if (ret)
    {
        initVertexNeighbors();
        this->updateTriangleNormals();
        this->updateVertexNormals();

        //edge information
        this->calcNeighborsVertices();
        this->calcEdges();
        this->upadateAABB();
    }

    return ret;
}

/// \brief --Deprecated, use loadMesh() for new simulators--
/// Loads the mesh based on the file type and initializes the normals
bool SurfaceMesh::loadMeshLegacy(const std::string& fileName, const MeshFileType &fileType)
{

    bool ret = true;

    switch (fileType)
    {
    case BaseMesh::MeshFileType::ThreeDS:
        Load3dsMesh(fileName);
        break;

    case BaseMesh::MeshFileType::Obj:
        ret = LoadMeshAssimp(fileName);
        break;

    default:
        if (log_SF != NULL)
        {
            log_SF->addError("Error: Mesh file TYPE UNIDENTIFIED");
        }

        ret = false;
    }

    meshFileType = fileType;
    assert(ret);

    if (ret == false)
    {
        if (log_SF != NULL)
        {
            log_SF->addError("Error: Mesh file NOT FOUND");
        }
    }

    if (ret)
    {
        initVertexNeighbors();
        this->updateTriangleNormals();
        this->updateVertexNormals();

        //edge information
        this->calcNeighborsVertices();
        this->calcEdges();
        this->upadateAABB();
    }

    return ret;
}

/// \brief
bool SurfaceMesh::LoadMeshAssimp(const std::string& fileName)
{

    //Tell Assimp to not import any of the following from the mesh it loads
    Assimp::Importer importer;
    importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS,
                                aiComponent_CAMERAS | aiComponent_LIGHTS |
                                aiComponent_MATERIALS | aiComponent_TEXTURES |
                                aiComponent_BONEWEIGHTS | aiComponent_COLORS |
                                aiComponent_TANGENTS_AND_BITANGENTS |
                                aiComponent_NORMALS | aiComponent_ANIMATIONS);

    //Import the file, and do some post-processing
    const aiScene* scene = importer.ReadFile(fileName,
                           aiProcess_Triangulate | //Triangulate any polygons that are not triangular
                           aiProcess_JoinIdenticalVertices | //Ensures indexed vertices from faces
                           aiProcess_RemoveComponent | //Removes the components in AI_CONFIG_PP_RVC_FLAGS
                           aiProcess_ImproveCacheLocality); //Reorders triangles for better vertex cache locality

    if (scene == NULL)
    {
        if (log_SF != NULL)
        {
            log_SF->addError("Error: Error loading mesh: " + std::string(fileName));
        }

        return false;
    }

    //extract the information from the aiScene's mesh objects
    aiMesh *mesh = scene->mMeshes[0]; //Guarenteed to have atleast one mesh

    if (mesh->HasTextureCoords(0))
    {
        this->isTextureCoordAvailable = 1;
    }
    else
    {
        this->isTextureCoordAvailable = 0;
    }

    initVertexArrays(mesh->mNumVertices);
    initTriangleArrays(mesh->mNumFaces);

    //Get indexed vertex data
    for (size_t i = 0; i < mesh->mNumVertices; i++)
    {
        this->vertices[i] = core::Vec3d(mesh->mVertices[i][0],
                                    mesh->mVertices[i][1],
                                    mesh->mVertices[i][2]);
    }
    this->origVerts = this->vertices;

    //Get indexed texture coordinate data
    if (isTextureCoordAvailable)
    {
        //Assimp supports 3D texture coords, but we only support 2D
        if (mesh->mNumUVComponents[0] != 2)
        {
            if (log_SF != NULL)
            {
                log_SF->addError("Error: Error loading mesh, non-two dimensional texture coordinate found.");
            }

            this->isTextureCoordAvailable = 0;
            return false;
        }

        //Extract the texture data
        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            this->texCoord[i].u = mesh->mTextureCoords[0][i][0];
            this->texCoord[i].v = mesh->mTextureCoords[0][i][1];
        }
    }

    //Setup triangle/face data
    for (size_t i = 0; i < mesh->mNumFaces; i++)
    {
        if (mesh->mFaces[i].mNumIndices != 3) //Make sure that the face is triangular
        {
            if (log_SF != NULL)
            {
                log_SF->addError("Error: Error loading mesh, non-triangular face found.");
            }

            //might want to consider an assert here also
            return false;
        }

        this->triangles[i].vert[0] = mesh->mFaces[i].mIndices[0];
        this->triangles[i].vert[1] = mesh->mFaces[i].mIndices[1];
        this->triangles[i].vert[2] = mesh->mFaces[i].mIndices[2];
    }

    return true;
}

/// \brief reads the mesh file in .3ds format
bool SurfaceMesh::Load3dsMesh(const std::string& fileName)
{

    int i; //Index variable
    FILE *l_file; //File pointer
    unsigned short l_chunk_id; //Chunk identifier
    unsigned int l_chunk_lenght; //Chunk lenght
    unsigned short l_qty; //Number of elements in each chunk
    unsigned short temp;
    char l_char;

    if ((l_file = fopen(fileName.c_str(), "rb")) == NULL) //Open the file
    {
        return 0;
    }

    while (size_t(ftell(l_file)) < Filelength(fileName.c_str(), fileno(l_file)))    //Loop to scan the whole file
    {

        fread(&l_chunk_id, 2, 1, l_file);  //Read the chunk header
        fread(&l_chunk_lenght, 4, 1, l_file);  //Read the lenght of the chunk

        switch (l_chunk_id)
        {
        //----------------- MAIN3DS -----------------
        // Description: Main chunk, contains all the other chunks
        // Chunk ID: 4d4d
        // Chunk Lenght: 0 + sub chunks
        //-------------------------------------------
        case 0x4d4d:
            break;

        //----------------- EDIT3DS -----------------
        // Description: 3D Editor chunk, objects layout info
        // Chunk ID: 3d3d (hex)
        // Chunk Lenght: 0 + sub chunks
        //-------------------------------------------
        case 0x3d3d:
            break;

        //--------------- EDIT_OBJECT ---------------
        // Description: Object block, info for each object
        // Chunk ID: 4000 (hex)
        // Chunk Lenght: len(object name) + sub chunks
        //-------------------------------------------
        case 0x4000:
            i = 0;

            do
            {
                fread(&l_char, 1, 1, l_file);
                i++;
            }
            while (l_char != '\0' && i < 20);

            break;

        //--------------- OBJ_TRIMESH ---------------
        // Description: Triangular mesh, contains chunks for 3d mesh info
        // Chunk ID: 4100 (hex)
        // Chunk Lenght: 0 + sub chunks
        //-------------------------------------------
        case 0x4100:
            break;

        //--------------- TRI_VERTEXL ---------------
        // Description: Vertices list
        // Chunk ID: 4110 (hex)
        // Chunk Lenght: 1 x unsigned short (number of vertices)
        //             + 3 x float (vertex coordinates) x (number of vertices)
        //             + sub chunks
        // Convert float to current real for precision
        //-------------------------------------------
        case 0x4110:
            fread(&l_qty, sizeof(unsigned short), 1, l_file);
            this->nbrVertices = l_qty;
            this->vertices.reserve(l_qty);
            this->origVerts.reserve(l_qty);
            this->vertNormals = new core::Vec3d[l_qty];
            this->vertTangents = new core::Vec3d[l_qty];
            this->texCoord = new TexCoord[l_qty];

            for (int fpt = 0; fpt < this->nbrVertices; fpt++)
            {
                float fTemp[3];
                fread(fTemp, sizeof(float), 3, l_file);
                this->vertices.emplace_back(fTemp[0], fTemp[1], fTemp[2]);
            }

            break;

        //--------------- TRI_FACEL1 ----------------
        // Description: Polygons (faces) list
        // Chunk ID: 4120 (hex)
        // Chunk Lenght: 1 x unsigned short (number of polygons)
        //             + 3 x unsigned short (polygon points) x (number of polygons)
        //             + sub chunks
        //-------------------------------------------
        case 0x4120:
            fread(&l_qty, sizeof(unsigned short), 1, l_file);
            this->nbrTriangles = l_qty;
            this->triangles = new Triangle[l_qty];
            this->triNormals = new core::Vec3d[l_qty];
            this->triTangents = new core::Vec3d[l_qty];

            for (i = 0; i < l_qty; i++)
            {
                fread(&temp, sizeof(unsigned short), 1, l_file);
                this->triangles[i].vert[0] = temp;

                fread(&temp, sizeof(unsigned short), 1, l_file);
                this->triangles[i].vert[1] = temp;

                fread(&temp, sizeof(unsigned short), 1, l_file);
                this->triangles[i].vert[2] = temp;
                fread(&temp, sizeof(unsigned short), 1, l_file);
            }

            break;

        //------------- TRI_MAPPINGCOORS ------------
        // Description: Vertices list
        // Chunk ID: 4140 (hex)
        // Chunk Lenght: 1 x unsigned short (number of mapping points)
        //             + 2 x float (mapping coordinates) x (number of mapping points)
        //             + sub chunks
        //-------------------------------------------
        case 0x4140:
            fread(&l_qty, sizeof(unsigned short), 1, l_file);

            for (int tpt = 0; tpt < l_qty; tpt++)
            {
                float fTemp[2];
                fread(fTemp, sizeof(float), 2, l_file);
                this->texCoord[tpt].u = fTemp[0];
                this->texCoord[tpt].v = fTemp[1];
            }

            isTextureCoordAvailable = true;

            break;

        //----------- Skip unknow chunks ------------
        //We need to skip all the chunks that currently we don't use
        //We use the chunk lenght information to set the file pointer
        //to the same level next chunk
        //-------------------------------------------
        default:
            fseek(l_file, l_chunk_lenght - 6, SEEK_CUR);
        }
    }
    this->origVerts = this->vertices;
    fclose(l_file);  // Closes the file stream

    return 1; // Returns ok
}
SurfaceMesh::SurfaceMesh()
{
    this->log_SF = std::shared_ptr<ErrorLog>();
    meshType = MeshType::Deformable;
    meshFileType = BaseMesh::MeshFileType::None;
}

void SurfaceMesh::printPrimitiveDetails()
{
    std::cout << "----------------------------\n";
    std::cout << "Mesh Info for   : " << this->getName() <<"\n\t";
    std::cout << "Num. vertices   : " << this->getNumVertices() <<"\n\t";
    std::cout << "Num. triangles  : " << this->getNumTriangles() << "\n\t";
    std::cout << "Num. edges      : " << this->getNumEdges() << "\n\t";
    std::cout << "Is mesh textured: " << this->isMeshTextured() << "\n";
    std::cout << "----------------------------\n";
}