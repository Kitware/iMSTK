/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkMshMeshIO.h"
#include "imstkHexahedralMesh.h"
#include "imstkLineMesh.h"
#include "imstkSurfaceMesh.h"
#include "imstkTetrahedralMesh.h"
#include "imstkVecDataArray.h"

#include <fstream>

namespace imstk
{
///
/// \brief Consume all characters up to delimiters
///
static void
readToDelimiter(std::ifstream& file)
{
    char next = file.peek();
    while (next == '\n' || next == ' ' || next == '\t' || next == '\r')
    {
        file.get();
        next = file.peek();
    }
}

///
/// \brief Converts a flat vector of ints to VecDataArray of ints of specified stride
///
template<int N>
static std::shared_ptr<VecDataArray<int, N>>
toVecDataArray(const std::vector<int>& vertIds)
{
    const int cellCount = vertIds.size() / N;
    CHECK(vertIds.size() % N == 0) << "Failed to convert array stride not divisable";
    std::shared_ptr<VecDataArray<int, N>> indicesPtr =
        std::make_shared<VecDataArray<int, N>>(cellCount);
    std::copy(vertIds.data(), vertIds.data() + cellCount * N,
        std::dynamic_pointer_cast<DataArray<int>>(indicesPtr)->getPointer());
    return indicesPtr;
}

std::shared_ptr<PointSet>
MshMeshIO::read(const std::string& filePath)
{
    // based on the format provided on (ASCII version)
    // ASCII: http://www.manpagez.com/info/gmsh/gmsh-2.2.6/gmsh_63.php
    // Binary: https://www.manpagez.com/info/gmsh/gmsh-2.4.0/gmsh_57.php

    std::shared_ptr<PointSet> results = nullptr;

    // Reopen as binary
    std::ifstream file;
    file.open(filePath, std::ios::binary | std::ios::in);
    CHECK(file.is_open()) << "Failed to read file, ifstream failed to open " << filePath;

    // Read $MeshFormat\n
    std::string bufferStr;
    file >> bufferStr;

    // Read version, type, dataSize (refers to floats/doubles)
    double version;
    int    fileType;
    int    dataSize;
    file >> version >> fileType >> dataSize;
    CHECK(dataSize == 8) << "Failed to read file, data size must be 8 bytes";
    CHECK(sizeof(int) == 4) << "Failed to read file, code must be compiled with int size 4 bytes";

    const bool isBinary = (fileType == 1);

    // Read the number one written in binary to check endianness
    // If it's not one then file was written with different endian
    if (isBinary)
    {
        int oneFromBinary;
        readToDelimiter(file);
        file.read(reinterpret_cast<char*>(&oneFromBinary), sizeof(int));
        CHECK(oneFromBinary == 1) << "Failed to read file, file saved with different endianness than this machine";
    }

    file >> bufferStr; // Read $EndMeshFormat
    CHECK(bufferStr == "$EndMeshFormat") << "Failed to read file, invalid format";

    CHECK(!file.fail()) << "Failed to read file, ifstream error";

    std::shared_ptr<VecDataArray<double, 3>> verticesPtr = nullptr;
    while (!file.eof())
    {
        bufferStr.clear();
        file >> bufferStr;
        CHECK(!file.fail()) << "Failed to read file, ifstream error";

        if (bufferStr == "$Nodes")
        {
            int nNodes = -1;
            file >> nNodes; // Read # of Nodes

            // Get the node IDs and the node coordinates
            verticesPtr = std::make_shared<VecDataArray<double, 3>>(nNodes);
            VecDataArray<double, 3>& vertices = *verticesPtr;
            std::vector<size_t>      nodeIDs(nNodes);

            if (isBinary)
            {
                // Read entire buffer as bytes
                size_t            numBytes = (4 + 3 * dataSize) * nNodes;
                std::vector<char> data(numBytes);
                readToDelimiter(file);
                file.read(data.data(), numBytes);
                for (int i = 0; i < nNodes; i++)
                {
                    int id = *reinterpret_cast<int*>(&data[i * (4 + 3 * dataSize)]) - 1;
                    nodeIDs[i] = id;

                    // Note in code above we restrict to only double (8 byte floating pt), but in
                    // the spec support could be added here for float (4 byte floating pt)

                    // Copy the next 3 doubles
                    double* x = reinterpret_cast<double*>(&data[i * (4 + 3 * dataSize) + 4]);
                    std::copy(x, x + 3, &vertices[id][0]);
                }
            }
            else
            {
                for (int i = 0; i < nNodes; i++)
                {
                    int   id;
                    Vec3d pos;
                    file >> id >> pos[0] >> pos[1] >> pos[2];
                    nodeIDs[i]       = id - 1;
                    vertices[id - 1] = pos;
                }
            }

            file >> bufferStr; // Read $EndNodes
            CHECK(bufferStr == "$EndNodes") << "Failed to read file, invalid format";
        }
        else if (bufferStr == "$Elements")
        {
            std::array<int, 6> elemTypeToCount =
            {
                0,
                2, // Line
                3, // Triangle
                4, // Quad
                4, // Tetrahedron
                8  // Hexahedron
            };

            // 1 - line, 2 - triangle, 3 - quad, 4 - tet, 5 - hex
            //std::array<std::vector<int>, 6> elementsIds;
            std::array<std::vector<int>, 6> elementVertIds;

            int numElements;
            file >> numElements;
            CHECK(!file.fail()) << "Failed to read file, ifstream error";
            readToDelimiter(file);

            if (isBinary)
            {
                int elemIter = 0;
                // Why not use a for loop?
                while (elemIter < numElements)
                {
                    // Parse element header.
                    int elemType, numElems, numTags;
                    file.read((char*)&elemType, sizeof(int));
                    file.read((char*)&numElems, sizeof(int));
                    file.read((char*)&numTags, sizeof(int));

                    CHECK(elemType > 0 && elemType < 6) <<
                        "Failed to read file, unsupported element type";
                    int               vertexCount = elemTypeToCount[elemType];
                    std::vector<int>& elemVertIds = elementVertIds[elemType];
                    //std::vector<int>& elemIds = elementsIds[elemType];

                    for (int i = 0; i < numElems; i++)
                    {
                        int elementId;
                        file.read((char*)&elementId, sizeof(int));
                        //elemIds.push_back(elementId - 1); // Msh starts from 1

                        // Read the tags but don't do anything with them
                        for (int j = 0; j < numTags; j++)
                        {
                            int tag;
                            file.read((char*)&tag, sizeof(int));
                        }

                        // Vertex ids
                        for (int j = 0; j < vertexCount; j++)
                        {
                            int vertId;
                            file.read((char*)&vertId, sizeof(int));
                            elemVertIds.push_back(vertId - 1);
                        }
                    }

                    elemIter += numElems;
                }
            }
            else
            {
                for (int i = 0; i < numElements; i++)
                {
                    // Parse element header.
                    int elemGroupId = -1;
                    int elemType    = -1;
                    int numTags     = 0;
                    file >> elemGroupId >> elemType >> numTags;
                    CHECK(elemType > 0 && elemType < 6) <<
                        "Failed to read file, unsupported element type";

                    int               vertexCount = elemTypeToCount[elemType];
                    std::vector<int>& elemVertIds = elementVertIds[elemType];

                    // Read the tags but don't do anything with them
                    for (int j = 0; j < numTags; j++)
                    {
                        int tag;
                        file >> tag;
                    }

                    // Vertex ids
                    for (int j = 0; j < vertexCount; j++)
                    {
                        int vertId;
                        file >> vertId;
                        elemVertIds.push_back(vertId - 1);
                    }
                }
            }

            // We only support homogenous element types
            int typeToUse = 0;
            int typeCount = 0;
            for (int i = 0; i < 6; i++)
            {
                if (elementVertIds[i].size() > 0)
                {
                    typeCount++;
                    typeToUse = i;
                }
            }
            // If we have more than one only choose the highest in vertex count of the element
            // so hex > tet > quad > tri > line
            if (typeCount > 1)
            {
                LOG(WARNING) << "MshMeshIO::read only supports homogenous types of elements, " <<
                    typeCount << " types of elements were found, choosing one";
            }

            if (typeToUse == 1)
            {
                auto mesh = std::make_shared<LineMesh>();
                mesh->initialize(verticesPtr, toVecDataArray<2>(elementVertIds[typeToUse]));
                results = mesh;
            }
            else if (typeToUse == 2)
            {
                auto mesh = std::make_shared<SurfaceMesh>();
                mesh->initialize(verticesPtr, toVecDataArray<3>(elementVertIds[typeToUse]));
                results = mesh;
            }
            else if (typeToUse == 4)
            {
                auto mesh = std::make_shared<TetrahedralMesh>();
                mesh->initialize(verticesPtr, toVecDataArray<4>(elementVertIds[typeToUse]));
                results = mesh;
            }
            else if (typeToUse == 5)
            {
                auto mesh = std::make_shared<HexahedralMesh>();
                mesh->initialize(verticesPtr, toVecDataArray<8>(elementVertIds[typeToUse]));
                results = mesh;
            }

            file >> bufferStr; // Read $EndElements
            CHECK(bufferStr == "$EndElements") << "Failed to read file, invalid format";

            // File is considered read after elements
            break;
        }
    }

    file.close();
    return results;
}
} // namespace imstk