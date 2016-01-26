// This file is part of the iMSTK project.
//
// Copyright (c) Kitware, Inc.
//
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

#include "Core/Factory.h"
#include "IO/IOMeshDelegate.h"
#include "Mesh/SurfaceMesh.h"

#ifdef _WIN32
    #define fileno _fileno
    #include <io.h>
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

namespace imstk {

class IOMesh3dsDelegate : public IOMeshDelegate
{
public:
    void read();
    void write(){}
};

//----------------------------------------------------------------------------------------
void IOMesh3dsDelegate::read()
{
    auto name = this->meshIO->getFileName().c_str();

    auto surfaceMesh = std::make_shared<SurfaceMesh>();
    auto &vertices = surfaceMesh->getVertices();
    auto &triangles = surfaceMesh->getTriangles();
    auto &texCoord = surfaceMesh->getTextureCoordinates();

    int i; //Index variable
    FILE *l_file; //File pointer
    unsigned short l_chunk_id; //Chunk identifier
    unsigned int l_chunk_lenght; //Chunk lenght
    unsigned short l_qty; //Number of elements in each chunk
    unsigned short temp;
    char l_char;

    if((l_file = fopen(name, "rb")) == nullptr)  //Open the file
    {
        std::cerr << "Mesh3dsDelegate: Can not open file: " << name << std::endl;
        return;
    }

    Vec3d v;
    std::array<size_t, 3> t;

    //Loop to scan the whole file
    while(size_t(ftell(l_file)) < Filelength(name, fileno(l_file)))
    {

        fread(&l_chunk_id, 2, 1, l_file);  //Read the chunk header
        fread(&l_chunk_lenght, 4, 1, l_file);  //Read the lenght of the chunk

        switch(l_chunk_id)
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
                    ++i;
                }
                while(l_char != '\0' && i < 20);

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
                vertices.reserve(l_qty);

                for(int fpt = 0; fpt < l_qty; ++fpt)
                {
                    fread(v.data(), sizeof(float), 3, l_file);
                    vertices.emplace_back(v);
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
                triangles.reserve(l_qty);

                for(i = 0; i < l_qty; ++i)
                {
                    fread(&temp, sizeof(unsigned short), 1, l_file);
                    t[0] = temp;

                    fread(&temp, sizeof(unsigned short), 1, l_file);
                    t[1] = temp;

                    fread(&temp, sizeof(unsigned short), 1, l_file);
                    t[2] = temp;

                    fread(&temp, sizeof(unsigned short), 1, l_file);
                    triangles.emplace_back(t);
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
                texCoord.reserve(l_qty);

                for(int tpt = 0; tpt < l_qty; ++tpt)
                {
                    float fTemp[2];
                    fread(fTemp, sizeof(float), 2, l_file);
                    texCoord[tpt][0] = fTemp[0];
                    texCoord[tpt][1] = fTemp[1];
                }

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

    fclose(l_file);  // Closes the file stream
    surfaceMesh->updateInitialVertices();
    this->meshIO->setMesh(surfaceMesh);
}

RegisterFactoryClass(IOMeshDelegate,IOMesh3dsDelegate,IOMesh::ReaderGroup::Other);

}
