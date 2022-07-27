/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkDynamicalModel.h"
#include "imstkImplicitFunctionFiniteDifferenceFunctor.h"

#include <unordered_map>
#include <tuple>

namespace imstk
{
class ImageData;

struct LevelSetModelConfig
{
    double m_dt = 0.001;             ///< Time step size
    bool m_sparseUpdate = false;     ///< Only updates nodes that recieve force
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
class LevelSetModel : public AbstractDynamicalModel
{
public:
    LevelSetModel();
    ~LevelSetModel() override = default;

    ///
    /// \brief Get/Set the time step size
    ///@{
    void setTimeStep(const double timeStep) override { m_config->m_dt = timeStep; }
    double getTimeStep() const override { return m_config->m_dt; }
    ///@}

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

    void resetToInitialState() override;

protected:
    ///
    /// \brief Setup the task graph of the LSM
    ///
    void initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink) override;

    std::shared_ptr<ImplicitGeometry> m_mesh = nullptr; ///< Geometry on which the levelset evolves with

    std::vector<std::shared_ptr<TaskNode>> m_evolveQuantitiesNodes;

    std::shared_ptr<TaskNode> m_generateVelocitiesBegin;
    std::shared_ptr<TaskNode> m_generateVelocitiesEnd;

    std::shared_ptr<LevelSetModelConfig> m_config;

    std::unordered_map<size_t, std::tuple<Vec3i, double>> m_nodesToUpdate;
    std::vector<std::tuple<size_t, Vec3i, double, Vec2d, double>> m_nodeUpdatePool;
    size_t noteUpdatePoolSize;
    size_t m_maxVelocitiesParallel = 100;                      // In sparse mode, if surpass this value, switch to parallel

    std::shared_ptr<ImageData> m_gradientMagnitudes = nullptr; ///< Gradient magnitude field when using dense
    std::shared_ptr<ImageData> m_velocities = nullptr;
    std::shared_ptr<ImageData> m_curvatures = nullptr;

    // I'm unable to use the more generic double/floating pt based version
    // suspect floating point error
    StructuredForwardGradient  m_forwardGrad;
    StructuredBackwardGradient m_backwardGrad;

    ImplicitStructuredCurvature m_curvature;
};
} // namespace imstk