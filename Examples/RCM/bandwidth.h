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

#pragma once

#include <vector>
#include <unordered_set>

///
/// \brief Build the vertex-to-vertex connectivity of a map
//
/// \param conn element-to-vertex connectivity of the map
/// \param numVerts number of vertice in the map
/// \retval vertToVert vertex-to-vertex connectivity
///
template<typename ElemConn>
static void
buildVertToVert(const std::vector<ElemConn>&             conn,
                const size_t                             numVerts,
                std::vector<std::unordered_set<size_t>>& vertToVert)
{
    // constexpr size_t numVertPerElem = ElemConn::size();
    std::vector<size_t> vertToElemPtr(numVerts + 1, 0);
    std::vector<size_t> vertToElem;

    // find the number of adjacent elements for each vertex
    for (const auto& vertices : conn)
    {
        for (auto vid : vertices)
        {
            vertToElemPtr[vid + 1] += 1;
        }
    }

    // accumulate pointer
    for (size_t i = 0; i < numVerts; ++i)
    {
        vertToElemPtr[i + 1] += vertToElemPtr[i];
    }

    // track the number
    auto   pos    = vertToElemPtr;
    size_t totNum = vertToElemPtr.back();

    vertToElem.resize(totNum);

    for (size_t eid = 0; eid < conn.size(); ++eid)
    {
        for (auto vid : conn[eid])
        {
            vertToElem[pos[vid]] = eid;
            ++pos[vid];
        }
    }

    // connectivity of vertex-to-vertex
    vertToVert.resize(numVerts);
    auto getVertexNbrs = [&vertToElem, &vertToElemPtr, &conn, &vertToVert](const size_t i) {
                             const auto ptr0 = vertToElemPtr[i];
                             const auto ptr1 = vertToElemPtr[i + 1];
                             size_t     eid;

                             for (auto ptr = ptr0; ptr < ptr1; ++ptr)
                             {
                                 eid = vertToElem[ptr];
                                 for (auto vid : conn[eid])
                                 {
                                     // vertex-i itself is also included.
                                     vertToVert[i].insert(vid);
                                 }
                             }
                         };

    for (size_t i = 0; i < numVerts; ++i)
    {
        getVertexNbrs(i);
    }
}

///
/// \brief Returns the bandwidth of a map
///
/// \param neighbors array of neighbors of each vertex; eg, neighbors[i] is a object containing
///
template<typename NBR>
size_t
bandwidth(const std::vector<NBR>& neighbors)
{
    size_t d    = 0;
    size_t dCur = 0;
    for (size_t i = 0; i < neighbors.size(); ++i)
    {
        for (const auto& j : neighbors[i])
        {
            dCur = (i > j) ? (i - j) : (j - i);
            d    = std::max(d, dCur);
        }
    }
    return d;
}

///
/// \brief Returns the bandwidth of a map
///
/// \param conn element-to-vertex connectivity of the map
/// \param numVerts number of vertices in the map
///
template<typename ElemConn>
size_t
bandwidth(const std::vector<ElemConn>& conn, const size_t numVerts)
{
    std::vector<std::unordered_set<size_t>> vertToVert;
    buildVertToVert(conn, numVerts, vertToVert);
    return bandwidth(vertToVert);
}
