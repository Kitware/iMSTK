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

#if 0
#include <array>
#include <memory>
#include <vector>

namespace imstk
{
struct InterFace
{
    size_t elemLeft;  ///< left element
    size_t elemRight;
    size_t faceLeft;  ///< face number in left element
    size_t faceRight;
};

struct Tetrahedron
{
    static constexpr int numVertices = 4;
    static constexpr int numFaces    = 4;
    static constexpr std::array<int, numFaces> numFaceVertices = { 3, 3, 3, 3 };
    // constexpr std::vector will be available in C++20
    static std::array<std::vector<int>, 4> faceVertices;
};

std::array<std::vector<int>, 4> Tetrahedron::faceVertices = { std::vector<int>{ 0, 2, 1 },
                                                              { 0, 1, 3 },
                                                              { 0, 3, 2 },
                                                              { 1, 2, 3 } };

struct Hexhedron
{
    static constexpr int numVertices = 8;
    static constexpr int numFaces    = 6;
    static constexpr std::array<int, numFaces> numFaceVertices = { 4, 4, 4, 4, 4, 4 };
    static std::array<std::vector<int>, numFaces> faceVertices;
};

std::array<std::vector<int>, 6> Hexhedron::faceVertices = { std::vector<int>{ 0, 2, 3, 1 },
                                                            { 4, 7, 6, 5 },
                                                            { 0, 4, 7, 3 },
                                                            { 1, 2, 6, 5 },
                                                            { 0, 1, 5, 4 },
                                                            { 2, 3, 7, 6 } };

///
/// \class MeshAdjacency
///
/// \brief Adjacency of a mesh
///
template<typename Element>
class MeshAdjacency
{
public:
    using ElemIndices = std::array<size_t, Element::numVertices>;
    MeshAdjacency(const std::vector<ElemIndices>& elemToVert, const size_t numVertices = 0)
        : m_numVertices(numVertices), m_elemToVert(elemToVert)
    {
    }

    ~MeshAdjacency() = default;
    MeshAdjacency& operator=(const MeshAdjacency&) = delete;

private:
    ///
    /// \brief Find adjacent elements for each vertex
    ///
    void m_buildVertexToElem();
    ///
    /// \brief Find the adjacent faces for each element
    ///
    void m_buildInterFace();

    /// \brief Given an element index and a face, get the indices of face vertex in the mesh
    /// \param eid element index
    /// \param fid index of the face in an element
    void m_getFaceVertices(const size_t eid, const int fid,
                           std::vector<size_t>& faceVertices) const;

private:
    size_t m_numVertices;
    const std::vector<ElemIndices>& m_elemToVert;
    std::vector<size_t>    m_vertToElem;
    std::vector<size_t>    m_vertToElem_ptr;
    std::vector<InterFace> interfaces;
};
}  // namespace imstk
#endif
