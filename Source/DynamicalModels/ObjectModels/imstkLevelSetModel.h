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

#include "imstkDynamicalModel.h"
#include "imstkLevelSetState.h"
#include "imstkImplicitFunctionFiniteDifferenceFunctor.h"

#include <unordered_map>
#include <tuple>

namespace imstk
{
class ImageData;

struct LevelSetModelConfig
{
    double m_dt = 0.001;             ///> Time step size
    bool m_sparseUpdate = false;     ///> Only updates nodes that recieve force
    bool m_useCurvature = false;
    double m_k = 0.05;               // Curvature term
    double m_constantVelocity = 0.0; // Constant velocity
    int m_substeps = 10;             // How many steps to do every iteration
};

///
/// \class LevelSetModel
///
/// \brief This class implements a generic level set model, it requires both a forward
/// and backward finite differencing method
///
class LevelSetModel : public DynamicalModel<LevelSetState>
{
public:
    ///
    /// \brief Constructor
    ///
    LevelSetModel();

    ///
    /// \brief Destructor
    ///
    virtual ~LevelSetModel() override = default;

public:
    ///
    /// \brief Set the time step size
    ///
    virtual void setTimeStep(const Real timeStep) override { m_config->m_dt = timeStep; }

    ///
    /// \brief Returns the time step size
    ///
    virtual double getTimeStep() const override { return m_config->m_dt; }

    std::shared_ptr<LevelSetModelConfig> getConfig() const { return m_config; }

    ///
    /// \brief Initialize the LevelSet model
    ///
    bool initialize() override;

    ///
    /// \brief Configure the model
    ///
    void configure(std::shared_ptr<LevelSetModelConfig> config);

    virtual void evolve();

    ///
    /// \brief Add an impulse to the velocity field of the levelset
    /// This actually takes the max of the current and provided
    ///
    void addImpulse(const Vec3i& coord, double f);
    ///
    /// \brief Set the impulse in the velocity field, replaces
    ///
    void setImpulse(const Vec3i& coord, double f);

    std::shared_ptr<TaskNode> getQuantityEvolveNode(size_t i) const { return m_evolveQuantitiesNodes[i]; }
    std::shared_ptr<TaskNode> getGenerateVelocitiesBeginNode() const { return m_generateVelocitiesBegin; }
    std::shared_ptr<TaskNode> getGenerateVelocitiesEndNode() const { return m_generateVelocitiesEnd; }

    std::unordered_map<size_t, std::tuple<Vec3i, double>>& getNodesToUpdate() { return m_nodesToUpdate; }

protected:
    ///
    /// \brief Setup the task graph of the LSM
    ///
    void initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink) override;

protected:
    std::shared_ptr<ImplicitGeometry> m_mesh = nullptr; ///> Geometry on which the levelset evolves with

    std::vector<std::shared_ptr<TaskNode>> m_evolveQuantitiesNodes;

    std::shared_ptr<TaskNode> m_generateVelocitiesBegin;
    std::shared_ptr<TaskNode> m_generateVelocitiesEnd;

    std::shared_ptr<LevelSetModelConfig> m_config;

    std::unordered_map<size_t, std::tuple<Vec3i, double>> m_nodesToUpdate;
    std::vector<std::tuple<size_t, Vec3i, double, Vec2d, double>> m_nodeUpdatePool;
    size_t noteUpdatePoolSize;

    std::shared_ptr<ImageData> m_gradientMagnitudes = nullptr; ///> Gradient magnitude field when using dense
    std::shared_ptr<ImageData> m_velocities = nullptr;
    std::shared_ptr<ImageData> m_curvatures = nullptr;

    // I'm unable to use the more generic double/floating pt based version
    // suspect floating point error
    StructuredForwardGradient  m_forwardGrad;
    StructuredBackwardGradient m_backwardGrad;

    ImplicitStructuredCurvature m_curvature;
};
}