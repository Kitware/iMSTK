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

#include "imstkMSHMeshIO.h"
#include "imstkHexahedralMesh.h"
#include "imstkLogger.h"
#include "imstkTetrahedralMesh.h"

#include <fstream>

namespace imstk
{
std::shared_ptr<imstk::VolumetricMesh>
MSHMeshIO::read(const std::string& filePath, const MeshFileType meshType)
{
    CHECK(meshType == MeshFileType::MSH) << "MSHMeshIO::read error: file type not supported";

    // based on the format provided on
    // http://www.manpagez.com/info/gmsh/gmsh-2.2.6/gmsh_63.php

    size_t                                   nNodes    = 0;           // number-of-nodes
    size_t                                   nElements = 0;           // number-of-elements
    std::vector<size_t>                      nodeIDs;                 // number assigned to each node (node number)
    StdVectorOfVec3d                         nodesCoords;             // nodes coordinates
    std::vector<size_t>                      tetrahedronIDs;          // tet elements IDs
    std::vector<size_t>                      hexahedronIDs;           // hex elements IDs
    std::vector<TetrahedralMesh::TetraArray> tetrahedronConnectivity; // tet element connectivity
    std::vector<HexahedralMesh::HexaArray>   hexahedronConnectivity;  // hex element connectivity
    std::map<ElemType, size_t>               elemCountMap;            // map of the element types to their number of counts
    std::string                              subString;               // to store space separated strings in a line
    std::string                              mshLine;                 // a msh file line
    std::stringstream                        mshLineStream;           // sting stream object which represent a line in the .msh file

    // Open the file
    std::ifstream mshStream(filePath);

    CHECK(mshStream.is_open()) << "Failed to open the input .msh file";

    // Look for "$MeshFormat"
    while (getline(mshStream, mshLine))
    {
        mshLineStream.str(std::string());
        mshLineStream.clear();
        if (!mshLine.empty())
        {
            mshLineStream << mshLine;
            mshLineStream >> subString;
            if (subString == "$MeshFormat")
            {
                break;
            }
        }
    }

    if (mshStream.eof())
    {
        LOG(INFO) << "Warning:  version number, file-type, data-size not found in the msh file.";
    }
    mshStream.clear();
    mshStream.seekg(0, std::ios::beg);

    // Look for "$NodeData"
    while (getline(mshStream, mshLine))
    {
        mshLineStream.str(std::string());
        mshLineStream.clear();
        if (!mshLine.empty())
        {
            mshLineStream << mshLine;
            mshLineStream >> subString;
            if (!subString.compare("$NOD") || !subString.compare("$Nodes"))
            {
                break;
            }
        }
    }
    if (mshStream.eof())
    {
        LOG(WARNING) << "MSHMeshIO::read error : Elements not defined in the file";
        return nullptr;
    }

    // Get the total number-of-nodes specified in $Node field
    while (getline(mshStream, mshLine))
    {
        if (!mshLine.empty())
        {
            mshLineStream.str(std::string());
            mshLineStream.clear();
            mshLineStream << mshLine;
            mshLineStream >> subString;
            nNodes = stoi(subString);
            break;
        }
    }
    LOG(INFO) << "The MSH mesh comprises of: \n" << '\t' << "Number of NODES: " << nNodes;

    // Get the node IDs and the node coordinates
    nodeIDs.resize(nNodes);
    std::string node_xC;
    std::string node_yC;
    std::string node_zC;
    size_t      nodes_count = 0;
    while (getline(mshStream, mshLine))
    {
        mshLineStream.str(std::string());
        mshLineStream.clear();
        if (!mshLine.empty())
        {
            mshLineStream << mshLine;
            mshLineStream >> subString;
            if (!subString.compare("$ENDNOD") || !subString.compare("$EndNodes"))
            {
                break;
            }
            nodeIDs[nodes_count] = stoul(subString);
            // x coordinate
            mshLineStream >> node_xC;
            // y coordinate
            mshLineStream >> node_yC;
            // z coordinate
            mshLineStream >> node_zC;
            nodesCoords.push_back(Vec3d { stod(node_xC), stod(node_yC), stod(node_zC) });
            ++nodes_count;
        }
    }

    // Check to the $Nodes field is in the correct format in .msh file
    if (nodes_count != nNodes)
    {
        LOG(WARNING) << " MSHMeshIO::read error: number of nodes read (" << nodes_count << ") "
                     << "inconsistent with number of nodes defined in file (" << nNodes << ").";
        return nullptr;
    }

    // Look for "$Elements" field
    mshStream.clear();
    mshStream.seekg(0, std::ios::beg);
    while (getline(mshStream, mshLine))
    {
        if (!mshLine.empty())
        {
            mshLineStream.str(std::string());
            mshLineStream.clear();
            mshLineStream << mshLine;
            mshLineStream >> subString;
            if (!subString.compare("$ELM") || !subString.compare("$Elements"))
            {
                break;
            }
        }
    }
    if (mshStream.eof())
    {
        LOG(WARNING) << "MSHMeshIO::read error : Elements not defined in the file";
        return nullptr;
    }

    // Get the total number-of-elements
    while (getline(mshStream, mshLine))
    {
        if (!mshLine.empty())
        {
            mshLineStream.str(std::string());
            mshLineStream.clear();
            mshLineStream << mshLine;
            mshLineStream >> subString;
            nElements = stoul(subString);
            break;
        }
    }

    // Get the total number of elements of each type
    int    elemType;                            // Store an element type
    size_t elemID;                              // Stores an element ID
    while (getline(mshStream, mshLine))
    {
        if (!mshLine.empty())
        {
            mshLineStream.str(std::string());
            mshLineStream.clear();
            mshLineStream << mshLine;
            mshLineStream >> subString;
            if (!subString.compare("$ENDELM") || !subString.compare("$EndElements"))
            {
                break;
            }
            //elemID = stoul(subString);  // Read the element ID
            subString.clear();
            mshLineStream >> subString; // Read the element type
            elemType = stoul(subString);

            // To avoid out of range casting, and to check validity of the .msh file
            if (elemType < ElemType::line || elemType > ElemType::tetrahedronFifthOrder)
            {
                LOG(FATAL) << "MSHMeshIO::read error : elm-type ( " << elemType << " ) "
                           << "is not in the range" << "(" << ElemType::line << " to "
                           << ElemType::tetrahedronFifthOrder << "), so is not a valid element type.";
                return nullptr;
            }
            ++elemCountMap[ElemType(elemType)];
        }
    }

    // Check to the $Elements field is in the correct format in .msh file
    size_t totalElem = 0;
    for (auto& kv : elemCountMap)
    {
        totalElem += kv.second;
    }

    // Set the stream back to the elem field
    if (!(nElements == totalElem))
    {
        LOG(WARNING) << "MSHMeshIO::read error: number of elements read (" << nElements << ") "
                     << "inconsistent with number of elements defined in file (" << totalElem << ").";
        return nullptr;
    }
    if (elemCountMap[ElemType::tetrahedron] == 0 && elemCountMap[ElemType::hexahedron] == 0)
    {
        LOG(WARNING) << "MSHMeshIO::read error: No tet or hex elements present in the mesh!";
        return nullptr;
    }

    // Read the tet and hex (if any) elements IDs and connectivity in the $Element field in the .msh file
    tetrahedronIDs.resize(elemCountMap[ElemType::tetrahedron]);
    hexahedronIDs.resize(elemCountMap[ElemType::hexahedron]);
    tetrahedronConnectivity.resize(elemCountMap[ElemType::tetrahedron]);
    hexahedronConnectivity.resize(elemCountMap[ElemType::hexahedron]);
    size_t                      tetElemCount = 0;
    size_t                      hexElemCount = 0;
    TetrahedralMesh::TetraArray tmp_4arr     = {};   // Temp array to store the connectivity of a tet element (if any)
    HexahedralMesh::HexaArray   tmp_8arr     = {};   // Temp array to store the connectivity of a hex element (if any)
    // Look for "$Elements" field
    mshStream.clear();
    mshStream.seekg(0, std::ios::beg);
    while (getline(mshStream, mshLine))
    {
        if (!mshLine.empty())
        {
            mshLineStream.str(std::string());
            mshLineStream.clear();
            mshLineStream << mshLine;
            mshLineStream >> subString;
            if (!subString.compare("$ELM") || !subString.compare("$Elements"))
            {
                getline(mshStream, mshLine); // To skipe the line specifying the total number of elements
                break;
            }
        }
    }
    while (getline(mshStream, mshLine))
    {
        if (!mshLine.empty())
        {
            mshLineStream.str(std::string());
            mshLineStream.clear();
            mshLineStream << mshLine;
            mshLineStream >> subString;
            if (!subString.compare("$ENDELM") || !subString.compare("$EndElements"))
            {
                break;
            }
            elemID = stoul(subString);  // Read the element ID
            subString.clear();
            mshLineStream >> subString; // Read the element type
            elemType = stoul(subString);
            // Reverse the string stream
            mshLineStream.str(std::string());
            mshLineStream.clear();
            reverse(mshLine.begin(), mshLine.end());
            mshLineStream << mshLine;
            switch (elemType)
            {
            case ElemType::tetrahedron: // for volumetric elements  (tets)
                tetrahedronIDs[tetElemCount] = elemID;
                for (size_t jj = numElemNodes(ElemType::tetrahedron); jj > 0; --jj)
                {
                    subString.clear();
                    mshLineStream >> subString;
                    reverse(subString.begin(), subString.end());
                    tmp_4arr[jj - 1] = stoul(subString);
                }
                tetrahedronConnectivity[tetElemCount] = tmp_4arr;
                ++tetElemCount;
                break;
            case ElemType::hexahedron: // for volumetric elements  (hexs)
                hexahedronIDs.push_back(elemID);
                for (size_t jj = numElemNodes(ElemType::hexahedron); jj > 0; --jj)
                {
                    subString.clear();
                    mshLineStream >> subString;
                    reverse(subString.begin(), subString.end());
                    tmp_8arr[jj - 1] = stoul(subString);
                }
                hexahedronConnectivity[hexElemCount] = tmp_8arr;
                ++hexElemCount;
                break;
            default:
                break;
            }
        }
    }
    mshStream.close(); // Close the file

    // Perform a manipulation to correct the node IDs (in case they are weirdly numbered).
    std::map<size_t, size_t> nodeIDMap;
    for (size_t iNode = 0; iNode < nNodes; ++iNode)
    {
        nodeIDMap.insert(std::pair<size_t, size_t>(nodeIDs[iNode], iNode));
    }

    // Generate iMSTK volumetric mesh
    if (elemCountMap[ElemType::tetrahedron] != 0)
    {
        std::vector<TetrahedralMesh::TetraArray> cells;
        for (size_t iTet = 0; iTet < elemCountMap[ElemType::tetrahedron]; ++iTet)
        {
            for (size_t jNode = 0; jNode < numElemNodes(ElemType::tetrahedron); ++jNode)
            {
                tetrahedronConnectivity[iTet][jNode] = nodeIDMap[tetrahedronConnectivity[iTet][jNode]];
            }
            cells.emplace_back(tetrahedronConnectivity[iTet]);
        }

        auto volMesh = std::make_shared<TetrahedralMesh>();
        volMesh->initialize(nodesCoords, cells, false);
        return volMesh;
    }
    else if (elemCountMap[ElemType::hexahedron] != 0)
    {
        std::vector<HexahedralMesh::HexaArray> cells;
        for (size_t iHex = 0; iHex < elemCountMap[ElemType::hexahedron]; ++iHex)
        {
            for (size_t jNode = 0; jNode < numElemNodes(ElemType::hexahedron); ++jNode)
            {
                hexahedronConnectivity[iHex][jNode] = nodeIDMap[hexahedronConnectivity[iHex][jNode]];
            }
            cells.emplace_back(hexahedronConnectivity[iHex]);
        }
        auto volMesh = std::make_shared<HexahedralMesh>();
        volMesh->initialize(nodesCoords, cells, false);
        return volMesh;
    }
    else
    {
        LOG(FATAL) << "This volume type is not supported in iMSTK";
        return nullptr;
    }
}

size_t
MSHMeshIO::numElemNodes(const ElemType& elType)
{
    switch (elType)
    {
    case ElemType::line:                            return 2;
    case ElemType::triangle:                        return 3;
    case ElemType::quadrangle:                      return 4;
    case ElemType::tetrahedron:                     return 4;
    case ElemType::hexahedron:                      return 8;
    case ElemType::prism:                           return 6;
    case ElemType::pyramid:                         return 5;
    case ElemType::lineSecondOrder:                 return 3;
    case ElemType::triangleSecondOrder:             return 6;
    case ElemType::quadrangleSecondOrderType1:      return 9;
    case ElemType::tetrahedronSecondOrder:          return 10;
    case ElemType::hexahedronSecondOrderType1:      return 27;
    case ElemType::prismSecondOrderType1:           return 18;
    case ElemType::pyramidSecondOrderType1:         return 14;
    case ElemType::point:                           return 1;
    case ElemType::quadrangleSecondOrderType2:      return 8;
    case ElemType::hexahedronSecondOrderType2:      return 20;
    case ElemType::prismSecondOrderType2:           return 15;
    case ElemType::pyramidSecondOrderType2:         return 13;
    case ElemType::triangleThirdOrderIncomplete:    return 9;
    case ElemType::triangleThirdOrder:              return 10;
    case ElemType::triangleFourthOrderIncomplete:   return 12;
    case ElemType::triangleFourthOrder:             return 15;
    case ElemType::triangleFifthOrderIncomplete:    return 15;
    case ElemType::triangleFifthOrder:              return 21;
    case ElemType::edgeThirdOrder:                  return 4;
    case ElemType::edgeFourthOrder:                 return 5;
    case ElemType::edgeFifthOrder:                  return 6;
    case ElemType::tetrahedronThirdOrder:           return 20;
    case ElemType::tetrahedronFourthOrder:          return 35;
    case ElemType::tetrahedronFifthOrder:           return 56;
    default:
        LOG(FATAL) << "MSHMeshIO::numElemNodes: Unknown element type";
        return 0;
    }
}
} // iMSTK