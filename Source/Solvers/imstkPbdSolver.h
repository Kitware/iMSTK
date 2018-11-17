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

#ifndef imstkPbdSolver_h
#define imstkPbdSolver_h

#include "imstkSolverBase.h"
#include "imstkPbdObject.h"
#include "imstkGraph.h"

namespace imstk
{
///
/// \class PbdSolver
///
/// \brief Position Based Dynamics solver
///
class PbdSolver : public SolverBase
{
public:
    ///
    /// \brief Constructors/Destructor
    ///
    PbdSolver() = default;
    ~PbdSolver() = default;

    PbdSolver(const PbdSolver &other) = delete;
    PbdSolver &operator=(const PbdSolver &other) = delete;

    ///
    /// \brief Set MaxIterations. The maximum number of nonlinear iterations.
    ///
    void setMaxIterations(const size_t newMaxIterations)
    {
        this->m_maxIterations = newMaxIterations;
    }

    ///
    /// \brief Get MaxIterations. Returns current maximum nonlinear iterations.
    ///
    size_t getMaxIterations() const
    {
        return this->m_maxIterations;
    }

    ///
    /// \brief
    ///
    void setPbdObject(const std::shared_ptr<PbdObject>& pbdObj)
    {
        m_pbdObject = pbdObj;
    }

    ///
    /// \brief Solve the non linear system of equations G(x)=0 using Newton's method.
    ///
    void solve()
    {
        m_pbdObject->integratePosition();
        m_pbdObject->solveConstraints();
        m_pbdObject->updateVelocity();
    }

    ///
    /// \brief Partition constraints for parallelization
    ///
    vector<vector<size_t>> partitionCostraints(const bool print = false)
    {
        auto pbdModel = std::dynamic_pointer_cast<PbdModel>(m_pbdObject->getDynamicalModel());
        const auto constraints = pbdModel->getConstraints();

        // Find the number of nodes involved
        size_t maxVertId = 0;
        for (const auto& constr : constraints)
        {
            auto vIds = constr->m_vertexIds;
            maxVertId = std::max(maxVertId, *max_element(begin(vIds), end(vIds)));
        }

        // Form the node-constraint adjacency list
        std::vector<std::vector<size_t>> adj(maxVertId+1);
        size_t constrId = 0;
        for (const auto& constr : constraints)
        {
            for (const auto& vIds : constr->m_vertexIds)
            {
                adj[vIds].push_back(constrId);
            }
            constrId++;
        }

        // Add edges to the constraint graph. Every constraint that depends on a particular node
        // forms an edge with other constraints that depend on the same node
        imstk::Graph constraintGraph(constraints.size());
        for (const auto& node : adj)
        {
            for (int i = 0; i < node.size(); i++)
            {
                for (int j = i + 1; j < node.size(); j++)
                {
                    constraintGraph.addEdge(node[i], node[j]);
                }
            }
        }
        adj.clear();

        // do graph coloring for the constraint graph
        auto coloredGraph = constraintGraph.doGreedyColoring();

        const auto numPartitions = *std::max_element(std::begin(coloredGraph), std::end(coloredGraph)) + 1;

        vector<vector<size_t>> partitions(numPartitions);
        size_t constrainId = 0;
        for (const auto& partition : coloredGraph)
        {
            partitions[partition].push_back(constrainId);
            constrainId++;
        }
        coloredGraph.clear();

        // print
        if (print)
        {
            size_t partitionId = 0;
            for (const auto& partition : partitions)
            {
                std::cout << "Partition # " << partitionId << " (# nodes: " << partition.size() << ")" << std::endl;
                for (const auto& node : partition)
                {
                    std::cout << " " << node;
                }
                std::cout << std::endl;
                partitionId++;
            }
        }

        return partitions;
    }

private:
    size_t m_maxIterations = 20;    ///< Maximum number of NL Gauss-Seidel iterations

    std::shared_ptr<PbdObject> m_pbdObject;
};
} // imstk

#endif // imstkPbdSolver_h