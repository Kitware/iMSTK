/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkDataArray.h"
#include "imstkImageData.h"
#include "imstkLevelSetModel.h"
#include "imstkLogger.h"
#include "imstkMacros.h"
#include "imstkParallelFor.h"
#include "imstkTaskGraph.h"

namespace imstk
{
LevelSetModel::LevelSetModel()
{
    // If given an image data
    m_validGeometryTypes = { "ImageData", "SignedDistanceField" };

    // Expresses a location to compute velocities, so other methods may know when velocities are done
    m_generateVelocitiesBegin = std::make_shared<TaskNode>(nullptr, "Compute Velocities Begin");
    m_generateVelocitiesEnd   = std::make_shared<TaskNode>(nullptr, "Compute Velocities End");

    // By default the level set defines a function for evolving the distances, this can be removed in subclasses
    m_evolveQuantitiesNodes.push_back(std::make_shared<TaskNode>(std::bind(&LevelSetModel::evolve, this), "Evolve Distances"));

    m_taskGraph->addNode(m_generateVelocitiesBegin);
    m_taskGraph->addNode(m_generateVelocitiesEnd);
    m_taskGraph->addNode(m_evolveQuantitiesNodes.back());
}

bool
LevelSetModel::initialize()
{
    if (m_geometry == nullptr)
    {
        LOG(WARNING) << "Levelset missing geometry";
        return false;
    }

    if (auto imageData = std::dynamic_pointer_cast<ImageData>(m_geometry))
    {
        if (imageData->getScalarType() != IMSTK_DOUBLE)
        {
            LOG(WARNING) << "Levelset only works with double image types";
            return false;
        }

        m_mesh = std::make_shared<SignedDistanceField>(imageData);
    }
    else
    {
        m_mesh = std::dynamic_pointer_cast<ImplicitGeometry>(m_geometry);
    }
    m_forwardGrad.setFunction(m_mesh);
    m_backwardGrad.setFunction(m_mesh);
    m_curvature.setFunction(m_mesh);

    if (auto sdf = std::dynamic_pointer_cast<SignedDistanceField>(m_mesh))
    {
        std::shared_ptr<ImageData> sdfImage = sdf->getImage();
        if (!m_config->m_sparseUpdate)
        {
            m_gradientMagnitudes = std::make_shared<ImageData>();
            m_gradientMagnitudes->allocate(IMSTK_DOUBLE, 2, sdfImage->getDimensions(), sdfImage->getSpacing(), sdfImage->getOrigin());

            /* m_curvatures = std::make_shared<ImageData>();
             m_curvatures->allocate(IMSTK_DOUBLE, 1, sdfImage->getDimensions(), sdfImage->getSpacing(), sdfImage->getOrigin());*/

            m_velocities = std::make_shared<ImageData>();
            m_velocities->allocate(IMSTK_DOUBLE, 1, sdfImage->getDimensions(), sdfImage->getSpacing(), sdfImage->getOrigin());
        }

        const Vec3d actualSpacing = sdf->getImage()->getSpacing();// *sdf->getScale();
        m_forwardGrad.setDx(Vec3i(1, 1, 1), actualSpacing);
        m_backwardGrad.setDx(Vec3i(1, 1, 1), actualSpacing);
        m_curvature.setDx(Vec3i(1, 1, 1), actualSpacing);
    }

    m_nodeUpdatePool.resize(5000);
    noteUpdatePoolSize = 0;

    return true;
}

void
LevelSetModel::configure(std::shared_ptr<LevelSetModelConfig> config)
{
    LOG_IF(FATAL, (!this->getModelGeometry())) << "LevelSetModel::configure - Set LevelSetModel geometry before configuration!";

    m_config = config;
}

void
LevelSetModel::evolve()
{
    auto         sdf       = std::dynamic_pointer_cast<SignedDistanceField>(m_mesh);
    auto         imageData = std::dynamic_pointer_cast<ImageData>(sdf->getImage());
    double*      imgPtr    = static_cast<double*>(imageData->getVoidPointer());
    const Vec3i& dim       = imageData->getDimensions();
    const double dt = m_config->m_dt / m_config->m_substeps;
    //const double k  = m_config->m_k;

    if (m_config->m_sparseUpdate)
    {
        // Sparse update
        if (m_nodesToUpdate.size() == 0)
        {
            return;
        }

        // Setup a map of 0 based index -> image sparse index m_nodesToUpdate to parallelize
        std::vector<size_t> baseIndexToImageIndex;
        baseIndexToImageIndex.reserve(m_nodesToUpdate.size());
        for (std::unordered_map<size_t, std::tuple<Vec3i, double>>::iterator iter = m_nodesToUpdate.begin(); iter != m_nodesToUpdate.end(); iter++)
        {
            baseIndexToImageIndex.push_back(iter->first);
        }
        DISABLE_WARNING_PUSH
        DISABLE_WARNING_PADDING

        // Compute gradients
        const double                             constantVel = m_config->m_constantVelocity;
        std::tuple<size_t, Vec3i, double, Vec2d> val;
        for (int j = 0; j < m_config->m_substeps; j++)
        {
            ParallelUtils::parallelFor(baseIndexToImageIndex.size(), [&](const size_t i)
                {
                    std::tuple<size_t, Vec3i, double, Vec2d, double>& outputVal = m_nodeUpdatePool[i];
                    const size_t& index = std::get<0>(outputVal) = baseIndexToImageIndex[i];

                    std::tuple<Vec3i, double>& inputVal = m_nodesToUpdate[index];

                    const Vec3i& coords    = std::get<1>(outputVal) = std::get<0>(inputVal);
                    std::get<2>(outputVal) = std::get<1>(inputVal);

                    // Gradients
                    const Vec3d gradPos = m_forwardGrad(Vec3d(coords[0], coords[1], coords[2]));
                    const Vec3d gradNeg = m_backwardGrad(Vec3d(coords[0], coords[1], coords[2]));

                    Vec3d gradNegMax = gradNeg.cwiseMax(0.0);
                    Vec3d gradNegMin = gradNeg.cwiseMin(0.0);
                    Vec3d gradPosMax = gradPos.cwiseMax(0.0);
                    Vec3d gradPosMin = gradPos.cwiseMin(0.0);

                    // Square them
                    gradNegMax = gradNegMax.cwiseProduct(gradNegMax);
                    gradNegMin = gradNegMin.cwiseProduct(gradNegMin);
                    gradPosMax = gradPosMax.cwiseProduct(gradPosMax);
                    gradPosMin = gradPosMin.cwiseProduct(gradPosMin);

                    const double posMag =
                        gradNegMax[0] + gradNegMax[1] + gradNegMax[2] +
                        gradPosMin[0] + gradPosMin[1] + gradPosMin[2];

                    const double negMag =
                        gradNegMin[0] + gradNegMin[1] + gradNegMin[2] +
                        gradPosMax[0] + gradPosMax[1] + gradPosMax[2];

                    std::get<3>(outputVal) = Vec2d(negMag, posMag);

                    // Curvature
                    //const double kappa = m_curvature(Vec3d(coords[0], coords[1], coords[2]));
                }, baseIndexToImageIndex.size() > 50);

            // Update levelset
            ParallelUtils::parallelFor(baseIndexToImageIndex.size(), [&](const size_t& i)
                {
                    const size_t index = std::get<0>(m_nodeUpdatePool[i]);
                    const double vel   = std::get<2>(m_nodeUpdatePool[i]) + constantVel;
                    const Vec2d& g     = std::get<3>(m_nodeUpdatePool[i]);
                    //const double kappa = std::get<4>(nodeUpdates[i]);

                    // If speed function positive use forward difference (posMag)
                    if (vel > 0.0)
                    {
                        imgPtr[index] += dt * (vel * std::sqrt(g[0]) /*+ kappa * k*/);
                    }
                    // If speed function negative use backward difference (negMag)
                    else if (vel < 0.0)
                    {
                        imgPtr[index] += dt * (vel * std::sqrt(g[1]) /*+ kappa * k*/);
                    }
            }, noteUpdatePoolSize > m_maxVelocitiesParallel);
        }
        DISABLE_WARNING_POP
        m_nodesToUpdate.clear();
    }
    else
    {
        // Dense update
        double* gradientMagPtr = static_cast<double*>(m_gradientMagnitudes->getScalars()->getVoidPointer());
        //double* curvaturesPtr = static_cast<double*>(m_curvatures->getScalars()->getVoidPointer());
        double* velocityMagPtr = static_cast<double*>(m_velocities->getScalars()->getVoidPointer());

        // Compute gradients
        ParallelUtils::parallelFor(dim[2],
            [&](const int& z)
            {
                int i = z * dim[0] * dim[1];
                for (int y = 0; y < dim[1]; y++)
                {
                    for (int x = 0; x < dim[0]; x++, i++)
                    {
                        // Gradients
                        const Vec3d gradPos = m_forwardGrad(Vec3d(x, y, z));
                        const Vec3d gradNeg = m_backwardGrad(Vec3d(x, y, z));
                        //curvaturesPtr[i] = m_curvature(Vec3d(x, y, z));

                        Vec3d gradNegMax = gradNeg.cwiseMax(0.0);
                        Vec3d gradNegMin = gradNeg.cwiseMin(0.0);
                        Vec3d gradPosMax = gradPos.cwiseMax(0.0);
                        Vec3d gradPosMin = gradPos.cwiseMin(0.0);

                        // Square them
                        gradNegMax = gradNegMax.cwiseProduct(gradNegMax);
                        gradNegMin = gradNegMin.cwiseProduct(gradNegMin);
                        gradPosMax = gradPosMax.cwiseProduct(gradPosMax);
                        gradPosMin = gradPosMin.cwiseProduct(gradPosMin);

                        // Pos
                        gradientMagPtr[i * 2 + 1] =
                            gradNegMax[0] + gradNegMax[1] + gradNegMax[2] +
                            gradPosMin[0] + gradPosMin[1] + gradPosMin[2];

                        // Neg
                        gradientMagPtr[i * 2] =
                            gradNegMin[0] + gradNegMin[1] + gradNegMin[2] +
                            gradPosMax[0] + gradPosMax[1] + gradPosMax[2];
                    }
                }
            });

        const double constantVel = m_config->m_constantVelocity;
        ParallelUtils::parallelFor(dim[0] * dim[1] * dim[2],
            [&](const int& i)
            {
                const double vel = constantVel + velocityMagPtr[i];
                // If speed function positive use forward difference
                if (constantVel > 0.0)
                {
                    imgPtr[i] += dt * (vel * std::sqrt(gradientMagPtr[i * 2]) /*+ curvaturesPtr[i] * k*/);
                }
                // If speed function negative use backward difference
                else if (constantVel < 0.0)
                {
                    imgPtr[i] += dt * (vel * std::sqrt(gradientMagPtr[i * 2 + 1]) /*+ curvaturesPtr[i] * k*/);
                }
            });
    }
}

void
LevelSetModel::addImpulse(const Vec3i& coord, double f)
{
    auto         sdf       = std::dynamic_pointer_cast<SignedDistanceField>(m_mesh);
    auto         imageData = std::dynamic_pointer_cast<ImageData>(sdf->getImage());
    const Vec3i& dim       = imageData->getDimensions();

    if (coord[0] >= 0 && coord[0] < dim[0]
        && coord[1] >= 0 && coord[1] < dim[1]
        && coord[2] >= 0 && coord[2] < dim[2])
    {
        const size_t index = coord[0] + coord[1] * dim[0] + coord[2] * dim[0] * dim[1];
        if (m_config->m_sparseUpdate)
        {
            if (m_nodesToUpdate.count(index) > 0)
            {
                m_nodesToUpdate[index] = std::tuple<Vec3i, double>(coord, std::max(std::get<1>(m_nodesToUpdate[index]), f));
            }
            else
            {
                m_nodesToUpdate[index] = std::tuple<Vec3i, double>(coord, f);
            }
        }
        else
        {
            double* velocitiesPtr = static_cast<double*>(m_velocities->getScalars()->getVoidPointer());
            velocitiesPtr[index] = std::max(velocitiesPtr[index], f);
        }
    }
}

void
LevelSetModel::setImpulse(const Vec3i& coord, double f)
{
    auto         sdf       = std::dynamic_pointer_cast<SignedDistanceField>(m_mesh);
    auto         imageData = std::dynamic_pointer_cast<ImageData>(sdf->getImage());
    const Vec3i& dim       = imageData->getDimensions();

    if (coord[0] >= 0 && coord[0] < dim[0]
        && coord[1] >= 0 && coord[1] < dim[1]
        && coord[2] >= 0 && coord[2] < dim[2])
    {
        const size_t index = coord[0] + coord[1] * dim[0] + coord[2] * dim[0] * dim[1];
        if (m_config->m_sparseUpdate)
        {
            m_nodesToUpdate[index] = std::tuple<Vec3i, double>(coord, f);
        }
        else
        {
            double* velocitiesPtr = static_cast<double*>(m_velocities->getScalars()->getVoidPointer());
            velocitiesPtr[index] = f;
        }
    }
}

void
LevelSetModel::resetToInitialState()
{
    // Due to having to store a copy of the initial image which is quite large reset is not implemented
    LOG(WARNING) << "LevelSetModel cannot reset";
}

void
LevelSetModel::initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink)
{
    m_taskGraph->addEdge(source, m_generateVelocitiesBegin);
    m_taskGraph->addEdge(m_generateVelocitiesBegin, m_generateVelocitiesEnd);

    // Given no fields are interacting all quantities should be able to update in parallel
    for (size_t i = 0; i < m_evolveQuantitiesNodes.size(); i++)
    {
        m_taskGraph->addEdge(m_generateVelocitiesEnd, m_evolveQuantitiesNodes[i]);
        m_taskGraph->addEdge(m_evolveQuantitiesNodes[i], sink);
    }
}
} // namespace imstk