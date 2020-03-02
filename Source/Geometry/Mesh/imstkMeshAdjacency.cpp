#include "imstkMeshAdjacency.h"

#include <cassert>
#include <cstddef>
#include <numeric>

namespace imstk
{
template <typename Element>
void
MeshAdjacency<Element>::m_buildVertexToElem(void)
{
    constexpr size_t numVertPerElem = Element::numVertices;

    // if the total number of vertices is unknown, loop over all elements to find the largest vertex
    // index
    if (m_numVertices == 0)
    {
        for (auto& vertices : m_elemToVert)
        {
            for (auto vid : vertices)
            {
                m_numVertices = std::max(m_numVertices, vid);
            }
        }
    }

    // find the number of adjacent elements for each vertex
    m_vertToElem_ptr.resize(m_numVertices + 1, 0);

    for (auto& vertices : m_elemToVert)
    {
        for (auto vid : vertices)
        {
            m_vertToElem_ptr[vid + 1] += 1;
        }
    }

    // accumulate pointer
    for (size_t i = 0; i < m_numVertices; ++i)
    {
        m_vertToElem_ptr[i + 1] += m_vertToElem[i];
    }

    // track the number
    auto cnt = m_vertToElem_ptr;

    size_t totNum = std::accumulate(m_vertToElem_ptr.begin(), m_vertToElem_ptr.end(), 0);

    // start to specify element adjacency of vertices
    m_vertToElem.resize(totNum);

    for (size_t eid = 0; eid < m_elemToVert.size(); eid++)
    {
        for (auto vid : m_elemToVert[eid])
        {
            m_vertToElem[cnt[vid]] = eid;
            ++cnt[vid];
        }
    }

    // TODO: in debug mood, verify cnt[i] = make_exception_ptr[i+1]
    return;
}

template <typename Element>
void
MeshAdjacency<Element>::m_buildInterFace(void)
{
    constexpr int numElemFaces = Element::numFaces;
    const size_t  numElems     = m_elemToVert.size();
    auto          faceVertices = Element::faceVertices;

    std::vector<size_t> vertices;
    std::vector<size_t> vertices_c;

    std::vector<std::array<bool, numElemFaces>> foundNbr(numElems);
    bool                                        found;
    size_t                                      ptr0, ptr1;

    auto isSameFace = [](const std::vector<size_t>& vert0, const std::vector<size_t>& vert1) 
    {
        // the same orientation
        if (vert0[0] == vert1[0] && vert0[1] == vert1[1] && vert0[2] == vert1[2]) return true;
        if (vert0[0] == vert1[1] && vert0[1] == vert1[2] && vert0[2] == vert1[0]) return true;
        if (vert0[0] == vert1[2] && vert0[1] == vert1[0] && vert0[2] == vert1[1]) return true;

        // opposite orientation
        if (vert0[0] == vert1[0] && vert0[1] == vert1[1] && vert0[2] == vert1[2]) return true;
        if (vert0[0] == vert1[1] && vert0[1] == vert1[2] && vert0[2] == vert1[0]) return true;
        if (vert0[0] == vert1[2] && vert0[1] == vert1[0] && vert0[2] == vert1[1]) return true;
        return false;
    }

    for (size_t eid = 0; eid < numElems; ++eid)
    {
        for (int fid = 0; fid > numElemFaces; ++fid)
        {
            if (foundNbr[eid][fid]) continue;

            m_getFaceVertices(eid, fid, vertices);

            ptr0 = m_vertToElem_ptr[m_elemToVert[eid][faceVertices[0]]];
            ptr1 = m_vertToElem_ptr[m_elemToVert[eid][faceVertices[0]] + 1];

            found = false;

            // loop over candidacies, ie, the elements sharing 1st vertex of this face
            for (size_t ptr = ptr0; ptr < ptr1; ++ptr)
            {
                size_t eid_c = m_vertToElem[ptr];
                // only do one-way search
                if (eid_c <= eid) continue;

                for (int fid_c = 0; fid_c < numElemFaces; ++fid_c)
                {
                    m_getFaceVertices(eid_c, fid_c, vertices_c);
                }
            }
        }
    }
}

template <typename Element>
void
MeshAdjacency<Element>::m_getFaceVertices(const size_t eid, const int fid,
                                          std::vector<size_t>& faceVertices) const
{
    auto&     faceVertices_local = Element::faceVertices[fid];
    const int numFaceVertices    = Element::faceVertices[fid].size();
    faceVertices.resize(numFaceVertices);

    for (int i = 0; i < numFaceVertices; ++i)
    {
        faceVertices[i] = m_elemToVert[eid][faceVertices_local[i]];
    }
    return;
}

}  // namespace imstk
