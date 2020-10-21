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
    double m_k = 0.2;                // Curvature term
    double m_constantVelocity = 0.0; // Constant velocity
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

    ///
    /// \brief Update body states given the newest update and the type of update
    ///
    virtual void updateBodyStates(const Vectord& /*q*/, const StateUpdateType /*updateType = stateUpdateType::displacement*/) override {}

    ///
    /// \brief Initialize the LevelSet model
    ///
    bool initialize() override;

    ///
    /// \brief Configure the model
    ///
    void configure(std::shared_ptr<LevelSetModelConfig> config);

    virtual void evolveDistanceField();

    void addImpulse(const Vec3i& coord, double f);
    void setImpulse(const Vec3i& coord, double f);

    std::shared_ptr<TaskNode> getQuantityEvolveNode(size_t i) const { return m_evolveQuantitiesNodes[i]; }

protected:
    ///
    /// \brief Setup the task graph of the LSM
    ///
    void initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink) override;

protected:
    std::shared_ptr<ImplicitGeometry> m_mesh = nullptr; ///> Geometry on which the levelset evolves with

    std::vector<std::shared_ptr<TaskNode>> m_evolveQuantitiesNodes;

    std::shared_ptr<LevelSetModelConfig> m_config;

    std::unordered_map<size_t, std::tuple<Vec3i, double>> nodesToUpdate;

    std::shared_ptr<ImageData> gradientMagnitudes = nullptr; ///> Gradient magnitude field when using dense

    // I'm unable to use the more generic double/floating pt based version
    // suspect floating point error
    StructuredForwardGradient  forwardGrad;
    StructuredBackwardGradient backwardGrad;
    /*ImplicitFunctionForwardGradient  forwardGrad;
    ImplicitFunctionBackwardGradient backwardGrad;*/
};
}