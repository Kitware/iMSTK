/*=========================================================================

    Library: iMSTK

    Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
    & Imaging in Medicine, Rensselaer Polytechnic Institute.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0.txt

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

    =========================================================================*/

#include <fstream>
#include <iostream>

#include "imstkMSHMeshIO.h"
#include "imstkTetrahedralMesh.h"

#include "tetMesh.h"

#include "g3log/g3log.hpp"

namespace imstk
{

std::shared_ptr<imstk::VolumetricMesh>
MSHMeshIO::read(const std::string & filePath, const MeshFileType meshType)
{
    if (meshType != MeshFileType::MSH)
    {
        LOG(WARNING) <<"MSHMeshIO::read error: file type not supported";
        return nullptr;
    }

    // based on the format provided on
    // http://www.manpagez.com/info/gmsh/gmsh-2.2.6/gmsh_63.php

    size_t Nnodes; // number of nodes
    std::vector<size_t> Nodes_tag;                                                 // number assigned to each node (node number)
    std::vector<double> Nodes_xCoor, Nodes_yCoor, Nodes_zCoor;                     // nodes coordinates
    size_t N1Delems = 0;                                                           // total number of lines
    size_t N2Delems = 0;                                                           // total number of surface elements (considering only triangle for now), cannot handle mix surface element types (ex. tri+quad, and so on).
    size_t N3Delems = 0;                                                           // total number of volumetric elements  (considering only tets for now)
    std::vector<size_t> N1Delems_tag;                                              // number assigned to each line element
    std::vector<size_t> N2Delems_tag;                                              // number assigned to each surface (triangle) element
    std::vector<size_t> N3Delems_tag;                                              // number assigned to each volumetric (tet) element
    std::vector<std::array<size_t,2>> N1Delems_conn;                               // line element connectivity
    std::vector<std::array<size_t,3>> N2Delems_conn;                               // triangle element connectivity
    std::vector<std::array<size_t,4>> N3Delems_conn;                               // tet element connectivity
    size_t OtherElems;                                                             // all element types other than line, triangle, tets.
    std::vector<int> tag_positions;

    std::ifstream msh_stream(filePath);
    if (!msh_stream.is_open())
    {
        LOG(WARNING) << "Failed to open the input .msh file";
        return nullptr;
    }

    std::string msh_line;
    std::stringstream msh_ss;
    std::string key_word;

    // look for "$MeshFormat"
    while (getline(msh_stream, msh_line))
    {
        msh_ss.str(std::string());
        msh_ss.clear();
        if (!msh_line.empty())
        {
            msh_ss << msh_line;
            msh_ss >> key_word;
            if (key_word == "$MeshFormat")
            {
                break;
            }
        }
    }

    if (msh_stream.eof())
    {
        LOG(INFO) << "Warning:  version number, file-type, data-size not found in the msh file.";
    }

    msh_stream.clear();
    msh_stream.seekg(0, std::ios::beg);

    // look for "$NodeData"
    while (getline(msh_stream, msh_line))
    {
        msh_ss.str(std::string());
        msh_ss.clear();
        if (!msh_line.empty())
        {
            msh_ss << msh_line;
            msh_ss >> key_word;

            if (!key_word.compare("$NOD") || !key_word.compare("$Nodes"))
            {
                break;
            }
        }
    }

    if (msh_stream.eof())
    {
        LOG(WARNING) << "Error: Nodes not defined." ;
        return nullptr;
    }
    else
    {
        std::string nnodes;
        while (getline(msh_stream, msh_line))
        {
            msh_ss.str(std::string());
            msh_ss.clear();
            if (!msh_line.empty())
            {
                msh_ss << msh_line;
                msh_ss >> nnodes;
                Nnodes = stoi(nnodes);
                break;
            }
        }
    }

    LOG(INFO) << "The MSH mesh comprises of: \n" << '\t' << "Number of NODES: " << Nnodes ;

    // get coordinates ( geometry )

    Nodes_tag.resize(Nnodes);
    Nodes_xCoor.resize(Nnodes);
    Nodes_yCoor.resize(Nnodes);
    Nodes_zCoor.resize(Nnodes);

    std::string node_tag;
    std::string node_xC;
    std::string node_yC;
    std::string node_zC;
    size_t nodes_count = 0;

    while (getline(msh_stream, msh_line))
    {
        msh_ss.str(std::string());
        msh_ss.clear();
        if (!msh_line.empty())
        {
            msh_ss << msh_line;
            msh_ss >> key_word;
            if (!key_word.compare("$ENDNOD") || !key_word.compare("$EndNodes"))
            {
                break;
            }
            else
            {
                msh_ss.str(std::string());
                msh_ss.clear();
                msh_ss << msh_line;
                msh_ss >> node_tag;
                Nodes_tag[nodes_count] = stoul(node_tag);
                // x coordinate
                msh_ss >> node_xC;
                Nodes_xCoor[nodes_count] = stod(node_xC);
                // y coordinate
                msh_ss >> node_yC;
                Nodes_yCoor[nodes_count] = stod(node_yC);
                // z coordinate
                msh_ss >> node_zC;
                Nodes_zCoor[nodes_count] = stod(node_zC);
                ++nodes_count;
            }
        }
    }

    if (nodes_count != Nnodes)
    {
        LOG(WARNING) << "Error in reading the nodes from the input MSH file.";
        return nullptr;
    }

    // Get the elements ( topology )
    msh_stream.clear();
    msh_stream.seekg(0, std::ios::beg);

    // look for "$ELM"
    while (getline(msh_stream, msh_line))
    {
        msh_ss.str(std::string());
        msh_ss.clear();
        if (!msh_line.empty())
        {
            msh_ss << msh_line;
            msh_ss >> key_word;
            if (!key_word.compare("$ELM") || !key_word.compare("$Elements"))
            {
                break;
            }
        }
    }

    if (msh_stream.eof())
    {
        LOG(WARNING) << "Error: Elements not defined.";
        return nullptr;
    }
    else
    {
        // get the total number of elements of each topology type
        size_t lineEl_count = 0;
        size_t surfEl_count = 0;
        size_t volEl_count = 0;
        size_t otherEl_count = 0;
        size_t elem_type;
        std::string temp;

        while (getline(msh_stream, msh_line))
        {
            msh_ss.str(std::string());
            msh_ss.clear();
            if (!msh_line.empty())
            {
                break;
            }
        }

        while (getline(msh_stream, msh_line))
        {
            msh_ss.str(std::string());
            msh_ss.clear();
            if (!msh_line.empty())
            {
                msh_ss << msh_line;
                msh_ss >> key_word;
                if (!key_word.compare("$ENDELM") || !key_word.compare("$EndElements"))
                {
                    break;
                }
                else
                {
                    msh_ss.str(std::string());
                    msh_ss.clear();
                    msh_ss << msh_line;
                    msh_ss >> temp;
                    msh_ss >> temp;
                    elem_type = stoul(temp);
                    if (elem_type == 1)
                    {
                        ++lineEl_count;
                    }
                    else if (elem_type == 2)
                    {
                        ++surfEl_count;
                    }
                    else if (elem_type == 3)
                    {
                        ++otherEl_count;
                    }
                    else if (elem_type == 4)
                    {
                        ++volEl_count;
                    }
                    else if (elem_type >= 5 && elem_type <= 32)
                    {
                        ++otherEl_count;
                    }
                    else
                    {
                        LOG(WARNING) << "Specified wrong element types.";
                        return nullptr;
                    }
                }
            }
        }
        N1Delems = lineEl_count;
        N2Delems = surfEl_count;
        N3Delems = volEl_count;
        OtherElems = otherEl_count;
    }

    // set the stream back to the elem field
    msh_stream.clear();
    msh_stream.seekg(0, std::ios::beg);

    while (getline(msh_stream, msh_line))
    {
        msh_ss.str(std::string());
        msh_ss.clear();
        if (!msh_line.empty())
        {
            msh_ss << msh_line;
            msh_ss >> key_word;
            if (!key_word.compare("$ELM") || !key_word.compare("$Elements"))
            {
                break;
            }
        }
    }

    getline(msh_stream, msh_line);
    std::string temp;
    msh_ss.str(std::string());
    msh_ss.clear();
    msh_ss << msh_line;
    msh_ss >> temp;

    if (!(stoul(temp) == (N1Delems + N2Delems + N3Delems + OtherElems)))
    {
        LOG(WARNING) << "Error reading the element field in the msh file .. exiting";
        return nullptr;
    }
    if (N3Delems == 0)
    {
        LOG(WARNING) << "No volumetric ( tetrahedral element) present in the msh file !" << '\n'
            << "Only creates vega format file for the volumetric meshes.. Exiting";
        return nullptr;
    }

    // initialize array to store the data
    N1Delems_tag.resize(N1Delems);
    N2Delems_tag.resize(N2Delems);
    N3Delems_tag.resize(N3Delems);

    std::string tmp_str;
    size_t elem_tag;
    size_t elem_type;
    size_t elem1D_counter = 0;
    size_t elem2D_counter = 0;
    size_t elem3D_counter = 0;
    std::array<size_t, 2> tmp_1arr;
    std::array<size_t, 3> tmp_2arr;
    std::array<size_t, 4> tmp_3arr;

    while (getline(msh_stream, msh_line))
    {
        msh_ss.str(std::string());
        msh_ss.clear();

        if (!msh_line.empty())
        {
            msh_ss << msh_line;
            msh_ss >> key_word;
            if (!key_word.compare("$ENDELM") || !key_word.compare("$EndElements"))
            {
                break;
            }
            else
            {
                msh_ss.str(std::string());
                msh_ss.clear();
                msh_ss << msh_line;
                msh_ss >> tmp_str;

                elem_tag = stoul(tmp_str);
                tmp_str.clear();
                msh_ss >> tmp_str;
                elem_type = stoul(tmp_str);

                msh_ss.str(std::string());
                msh_ss.clear();

                reverse(msh_line.begin(), msh_line.end());
                msh_ss << msh_line;

                switch (elem_type)
                {
                    case 1: // for lines
                        N1Delems_tag[elem1D_counter] = elem_tag;
                        for (int jj = 1; jj >= 0; --jj)
                        {
                            tmp_str.clear();
                            msh_ss >> tmp_str;
                            reverse(tmp_str.begin(), tmp_str.end());
                            tmp_1arr[jj] = stoul(tmp_str);
                        }
                        N1Delems_conn.emplace_back(tmp_1arr);
                        ++elem1D_counter;
                        break;
                    case 2: // for surface elements (triangles)
                        N2Delems_tag[elem2D_counter] = elem_tag;
                        for (int jj = 2; jj >= 0; --jj)
                        {
                            tmp_str.clear();
                            msh_ss >> tmp_str;
                            reverse(tmp_str.begin(), tmp_str.end());
                            tmp_2arr[jj] = stoul(tmp_str);
                        }
                        N2Delems_conn.emplace_back(tmp_2arr);
                        ++elem2D_counter;
                        break;
                    case 4: // for volumetric elements  (tets)
                        N3Delems_tag[elem3D_counter] = elem_tag;
                        for (int jj = 3; jj >= 0; --jj)
                        {
                            tmp_str.clear();
                            msh_ss >> tmp_str;
                            reverse(tmp_str.begin(), tmp_str.end());
                            tmp_3arr[jj] = stoul(tmp_str);
                        }
                        N3Delems_conn.emplace_back(tmp_3arr);
                        ++elem3D_counter;
                        break;
                    default:
                        break;
                }
            }
        }
    }
    msh_stream.close();

    // perform some manipulation to correct the node tags.
    size_t tag_max = 0;
    for (size_t ii = 0; ii < Nnodes; ++ii)
    {
        if (Nodes_tag[ii] > tag_max)
        {
            tag_max = Nodes_tag[ii];
        }
    }
    tag_positions.assign(tag_max, -1);
    for (size_t ii = 0; ii < Nnodes; ++ii)
    {
        tag_positions[Nodes_tag[ii] - 1] = static_cast<int>(ii); // static cast only to avoid warning
    }
    for (size_t ii = 0; ii < N3Delems; ++ii)
    {
        for (size_t jj = 0; jj < 4; ++jj)
        {
            N3Delems_conn[ii][jj] = tag_positions[N3Delems_conn[ii][jj] - 1];
        }
    }

    // generate volumetric mesh
    StdVectorOfVec3d vertices;
    for (size_t ii = 0; ii < Nnodes; ++ii)
    {
        vertices.emplace_back(Nodes_xCoor[ii], Nodes_yCoor[ii], Nodes_zCoor[ii]);
    }
    std::vector<TetrahedralMesh::TetraArray> cells;
    for (size_t ii = 0; ii < N3Delems; ++ii)
    {
        cells.emplace_back(N3Delems_conn[ii]);
    }
    auto tetMesh = std::make_shared<TetrahedralMesh>();
    tetMesh->initialize(vertices, cells, false);
    return tetMesh;
};

}