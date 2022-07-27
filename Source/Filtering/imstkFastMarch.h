/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkImageData.h"

#include <unordered_set>
#include <queue>

namespace imstk
{
///
/// \class FastMarch
///
/// \brief
///
class FastMarch
{
protected:
    // Setup Node struct for priority queue
    struct Node
    {
        int m_nodeId;
        double m_cost;
        Vec3i m_coord;

        Node(int nodeId, double cost, Vec3i coord) :
            m_nodeId(nodeId), m_cost(cost), m_coord(coord)
        {
        }
    };
    struct NodeComparator
    {
        bool operator()(const Node& a, const Node& b)
        {
            return a.m_cost > b.m_cost;
        }
    };

public:
    bool isVisited(int nodeId) { return m_visited.count(nodeId) == 0 ? false : true; }
    double getDistance(int nodeId) { return m_distances.count(nodeId) == 0 ? IMSTK_DOUBLE_MAX : m_distances.at(nodeId); }

    void solve();

    void solveNode(Vec3i coord, int index);

    void setSeeds(std::vector<Vec3i> seedVoxels) { m_seedVoxels = seedVoxels; }
    void setVisited(std::unordered_set<int> visited) { m_visited = visited; }
    void setDistnaces(std::unordered_map<int, double> distances) { m_distances = distances; }
    void setImage(std::shared_ptr<ImageData> image) { m_imageData = image; }
    void setDistThreshold(double distThreshold) { m_distThreshold = distThreshold; }

protected:
    // The image to operate on
    std::shared_ptr<ImageData> m_imageData;
    Vec3i m_dim;
    Vec3d m_spacing;
    int   m_indexShift;

    std::unordered_set<int> m_visited;
    std::unordered_map<int, double> m_distances;

    // The starting voxels
    std::vector<Vec3i> m_seedVoxels;

    // Distance to go too
    double m_distThreshold;

    std::priority_queue<Node, std::vector<Node>, NodeComparator> m_queue;
};
} // namespace imstk