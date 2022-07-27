/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkFastMarch.h"
#include "imstkLogger.h"
#include "imstkVecDataArray.h"

namespace imstk
{
void
FastMarch::solve()
{
    // Get the scalars (ensure they're single component doubles)
    std::shared_ptr<AbstractDataArray> abstractScalars = m_imageData->getScalars();
    if (abstractScalars->getScalarType() != IMSTK_DOUBLE || abstractScalars->getNumberOfComponents() != 1)
    {
        LOG(WARNING) << "fastMarch only works with single component double images";
        return;
    }
    auto    scalars = std::dynamic_pointer_cast<DataArray<double>>(abstractScalars);
    double* imgPtr  = scalars->getPointer();
    m_dim        = m_imageData->getDimensions();
    m_spacing    = m_imageData->getSpacing();
    m_indexShift = m_dim[0] * m_dim[1];

    // We maintain a solution in maps, to keep things sparse

    // Sparse container for which nodes are marked visited
    m_visited = std::unordered_set<int>();
    // Sparse container for nodal distances
    m_distances = std::unordered_map<int, double>();   // Solved distances

    m_queue = std::priority_queue<Node, std::vector<Node>, NodeComparator>();

    // Add the initial seeds to the queue
    for (size_t i = 0; i < m_seedVoxels.size(); i++)
    {
        const Vec3i& coord = m_seedVoxels[i];
        if (coord[0] < 0 || coord[0] >= m_dim[0]
            || coord[1] < 0 || coord[1] >= m_dim[1]
            || coord[2] < 0 || coord[2] >= m_dim[2])
        {
            continue;
        }
        const int index = static_cast<int>(m_imageData->getScalarIndex(coord));
        m_distances[index] = imgPtr[m_imageData->getScalarIndex(coord)];
        m_queue.push(Node(index, 0.0, coord));
    }

    // Process every node in order of minimum distance
    while (!m_queue.empty())
    {
        Node node = m_queue.top();
        m_queue.pop();

        const Vec3i& coord  = node.m_coord;
        const int&   nodeId = node.m_nodeId;

        // Termination conditions
        if (isVisited(nodeId)
            || getDistance(nodeId) >= m_distThreshold)
        {
            continue;
        }

        // Mark node as visited (to avoid readdition)
        m_visited.insert(nodeId);

        // Update all its neighbor cells (diagonals not considered neighbors)
        // Right +x
        int   neighborId    = nodeId + 1;
        Vec3i neighborCoord = coord + Vec3i(1, 0, 0);
        if (neighborCoord[0] < m_dim[0] && !isVisited(neighborId))
        {
            solveNode(neighborCoord, neighborId);
        }
        // Left -x
        neighborId    = nodeId - 1;
        neighborCoord = coord - Vec3i(1, 0, 0);
        if (neighborCoord[0] >= 0 && !isVisited(neighborId))
        {
            solveNode(neighborCoord, neighborId);
        }

        // Up +y
        neighborId    = nodeId + m_dim[0];
        neighborCoord = coord + Vec3i(0, 1, 0);
        if (neighborCoord[1] < m_dim[1] && !isVisited(neighborId))
        {
            solveNode(neighborCoord, neighborId);
        }
        // Down -y
        neighborId    = nodeId - m_dim[0];
        neighborCoord = coord - Vec3i(0, 1, 0);
        if (neighborCoord[1] >= 0 && !isVisited(neighborId))
        {
            solveNode(neighborCoord, neighborId);
        }

        // Forward +z
        neighborId    = nodeId + m_indexShift;
        neighborCoord = coord + Vec3i(0, 0, 1);
        if (neighborCoord[2] < m_dim[2] && !isVisited(neighborId))
        {
            solveNode(neighborCoord, neighborId);
        }
        // Backward -z
        neighborId    = nodeId - m_indexShift;
        neighborCoord = coord - Vec3i(0, 0, 1);
        if (neighborCoord[2] >= 0 && !isVisited(neighborId))
        {
            solveNode(neighborCoord, neighborId);
        }
    }

    // Write the sparse distances to the image
    for (auto i : m_distances)
    {
        imgPtr[i.first] = i.second;
    }
}

void
FastMarch::solveNode(Vec3i coord, int index)
{
    // Compute the min distance in each axes
    const double dists[6] =
    {
        coord[0] - 1 >= 0 ? getDistance(index - 1) : IMSTK_DOUBLE_MAX,
        coord[0] + 1 < m_dim[0] ? getDistance(index + 1) : IMSTK_DOUBLE_MAX,
        coord[1] - 1 >= 0 ? getDistance(index - m_dim[0]) : IMSTK_DOUBLE_MAX,
        coord[1] + 1 < m_dim[1] ? getDistance(index + m_dim[0]) : IMSTK_DOUBLE_MAX,
        coord[2] - 1 >= 0 ? getDistance(index - m_indexShift) : IMSTK_DOUBLE_MAX,
        coord[2] + 1 < m_dim[2] ? getDistance(index + m_indexShift) : IMSTK_DOUBLE_MAX
    };
    const double minDist[3] =
    {
        std::min(dists[0], dists[1]),
        std::min(dists[2], dists[3]),
        std::min(dists[4], dists[5])
    };

    // Sort so that the min of minDist is first
    int dimReorder[3] = { 0, 1, 2 };
    for (int i = 0; i < 3; i++)
    {
        for (int j = i + 1; j < 3; j++)
        {
            const int dim1 = dimReorder[i];
            const int dim2 = dimReorder[j];
            if (minDist[dim1] > minDist[dim2])
            {
                std::swap(dimReorder[i], dimReorder[j]);
            }
        }
    }

    double aa = 0.0;
    double bb = 0.0;
    double cc = -1.0;

    // For every dimension
    double solution = IMSTK_DOUBLE_MAX;
    double discrim  = 0.0;
    // todo: Sort dimensions by minDist, start with smallest (break faster)
    for (unsigned int i = 0; i < 3; i++)
    {
        const double value = minDist[dimReorder[i]];
        if (solution >= value)
        {
            const double spaceFactor = std::sqrt(1.0 / m_spacing[dimReorder[i]]);
            aa += spaceFactor;
            bb += value * spaceFactor;
            cc += value * value * spaceFactor;

            discrim = bb * bb - aa * cc;
            if (discrim < 0.0)
            {
                // Whoops
                return;
            }

            solution = (std::sqrt(discrim) + bb) / aa;
        }
        else
        {
            break;
        }
    }

    if (solution < IMSTK_DOUBLE_MAX)
    {
        // Accept it as the new distance
        m_distances[index] = solution;
        m_queue.push(Node(index, solution, coord));
    }
}
} // namespace imstk