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

#include "imstkLevelSetModel.h"
#include "imstkTaskGraph.h"
#include "imstkGeometryUtilities.h"
#include "imstkLogger.h"
#include "imstkSignedDistanceField.h"
#include "imstkParallelFor.h"
#include "imstkImplicitGeometry.h"
#include "imstkDataArray.h"
#include "imstkImageData.h"

#include <vtkImplicitPolyDataDistance.h>
#include <vtkPolyData.h>

namespace imstk
{
LevelSetModel::LevelSetModel()
{
    // If given an image data
    m_validGeometryTypes = {
        Geometry::Type::ImageData,
        Geometry::Type::SignedDistanceField
    };

    // By default the level set defines a function for evolving the distances, this can be removed in subclasses
    m_evolveQuantitiesNodes.push_back(std::make_shared<TaskNode>(std::bind(&LevelSetModel::evolveDistanceField, this), "Evolve Distances"));
    m_taskGraph->addNode(m_evolveQuantitiesNodes.back());
    /// are all quantities evolved the same but with different force functions?
}

bool
LevelSetModel::initialize()
{
    if (m_geometry == nullptr)
    {
        LOG(WARNING) << "Levelset missing geometry";
        return false;
    }

    if (m_geometry->getType() == Geometry::Type::ImageData)
    {
        if (std::dynamic_pointer_cast<ImageData>(m_geometry)->getScalarType() != IMSTK_DOUBLE)
        {
            LOG(WARNING) << "Levelset only works with double image types";
            return false;
        }

        m_mesh = std::make_shared<SignedDistanceField>(std::dynamic_pointer_cast<ImageData>(m_geometry));
    }
    else
    {
        m_mesh = std::dynamic_pointer_cast<ImplicitGeometry>(m_geometry);
    }
    m_forwardGrad.setFunction(m_mesh);
    m_backwardGrad.setFunction(m_mesh);
    m_curvature.setFunction(m_mesh);

    // If dense update, we need a gradient image, which will store forward and backward gradient magnitudes
    if (m_mesh->getType() == Geometry::Type::SignedDistanceField && !m_config->m_sparseUpdate)
    {
        auto sdfImage = std::dynamic_pointer_cast<SignedDistanceField>(m_mesh)->getImage();

        m_gradientMagnitudes = std::make_shared<ImageData>();
        m_gradientMagnitudes->allocate(IMSTK_DOUBLE, 2, sdfImage->getDimensions(), sdfImage->getSpacing(), sdfImage->getOrigin());

        m_curvatures = std::make_shared<ImageData>();
        m_curvatures->allocate(IMSTK_DOUBLE, 1, sdfImage->getDimensions(), sdfImage->getSpacing(), sdfImage->getOrigin());

        m_velocities = std::make_shared<ImageData>();
        m_velocities->allocate(IMSTK_DOUBLE, 1, sdfImage->getDimensions(), sdfImage->getSpacing(), sdfImage->getOrigin());
        //std::fill_n(static_cast<double*>(m_velocities->getScalars()->getVoidPointer()), m_velocities->get
    }

    m_forwardGrad.setDx(Vec3i(1, 1, 1), std::dynamic_pointer_cast<SignedDistanceField>(m_mesh)->getImage()->getSpacing());
    m_backwardGrad.setDx(Vec3i(1, 1, 1), std::dynamic_pointer_cast<SignedDistanceField>(m_mesh)->getImage()->getSpacing());
    /*forwardGrad.setDx(std::dynamic_pointer_cast<SignedDistanceField>(m_mesh)->getImage()->getSpacing());
    backwardGrad.setDx(std::dynamic_pointer_cast<SignedDistanceField>(m_mesh)->getImage()->getSpacing());*/
    m_curvature.setDx(Vec3i(1, 1, 1), std::dynamic_pointer_cast<SignedDistanceField>(m_mesh)->getImage()->getSpacing());

    return true;
}

void
LevelSetModel::configure(std::shared_ptr<LevelSetModelConfig> config)
{
    LOG_IF(FATAL, (!this->getModelGeometry())) << "LevelSetModel::configure - Set LevelSetModel geometry before configuration!";

    m_config = config;
}

void
LevelSetModel::evolveDistanceField()
{
    auto         sdf       = std::dynamic_pointer_cast<SignedDistanceField>(m_mesh);
    auto         imageData = std::dynamic_pointer_cast<ImageData>(sdf->getImage());
    double*      imgPtr    = static_cast<double*>(imageData->getVoidPointer());
    const Vec3i& dim       = imageData->getDimensions();
    //const Vec3d& spacing   = imageData->getSpacing();
    //const Vec3d& origin    = imageData->getOrigin();
    const double dt = m_config->m_dt;
    const double k  = m_config->m_k;

    if (m_config->m_sparseUpdate)
    {
        // Sparse update
        if (m_nodesToUpdate.size() == 0)
        {
            return;
        }

        // index, coordinates, force, forward/backward gradient magnitude, curvature
        std::vector<std::tuple<size_t, Vec3i, double, Vec2d, double>> nodeUpdates;
        nodeUpdates.reserve(m_nodesToUpdate.size());

        // Compute gradients
        for (std::unordered_map<size_t, std::tuple<Vec3i, double>>::iterator iter = m_nodesToUpdate.begin(); iter != m_nodesToUpdate.end(); iter++)
        {
            const size_t index  = iter->first;
            const Vec3i& coords = std::get<0>(iter->second);
            const double f      = std::get<1>(iter->second);

            // Gradients
            const Vec3d gradPos = m_forwardGrad(Vec3d(coords[0], coords[1], coords[2]));
            const Vec3d gradNeg = m_backwardGrad(Vec3d(coords[0], coords[1], coords[2]));

            const double posMag =
                std::pow(std::max(gradNeg[0], 0.0), 2) + std::pow(std::min(gradPos[0], 0.0), 2) +
                std::pow(std::max(gradNeg[1], 0.0), 2) + std::pow(std::min(gradPos[1], 0.0), 2) +
                std::pow(std::max(gradNeg[2], 0.0), 2) + std::pow(std::min(gradPos[2], 0.0), 2);

            const double negMag =
                std::pow(std::min(gradNeg[0], 0.0), 2) + std::pow(std::max(gradPos[0], 0.0), 2) +
                std::pow(std::min(gradNeg[1], 0.0), 2) + std::pow(std::max(gradPos[1], 0.0), 2) +
                std::pow(std::min(gradNeg[2], 0.0), 2) + std::pow(std::max(gradPos[2], 0.0), 2);

            // Curvature
            //const double kappa = m_curvature(Vec3d(coords[0], coords[1], coords[2]));

            nodeUpdates.push_back(std::tuple<size_t, Vec3i, double, Vec2d, double>(index, coords, f, Vec2d(negMag, posMag), 0.0));
        }

        // Update levelset
        const double constantVel = m_config->m_constantVelocity;
        for (size_t i = 0; i < nodeUpdates.size(); i++)
        {
            const size_t index = std::get<0>(nodeUpdates[i]);
            //const Vec3i& coords = std::get<1>(nodeUpdates[i]);
            const double vel = std::get<2>(nodeUpdates[i]) + constantVel;
            const Vec2d& g   = std::get<3>(nodeUpdates[i]);
            //const double kappa = std::get<4>(nodeUpdates[i]);

            // If speed function positive use forward difference (posMag)
            if (vel > 0.0)
            {
                imgPtr[index] += dt * (vel * std::sqrt(g[0]) /*+ kappa*/);
            }
            // If speed function negative use backward difference (negMag)
            else if (vel < 0.0)
            {
                imgPtr[index] += dt * (vel * std::sqrt(g[1]) /*+ kappa * k*/);
            }
        }
        if (m_nodesToUpdate.size() > 0)
        {
            m_nodesToUpdate.clear();
        }
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
                        // Center of voxel
                        //const Vec3d pos = Vec3d(x, y, z).cwiseProduct(spacing) + shift;

                        // Gradients
                        const Vec3d gradPos = m_forwardGrad(Vec3d(x, y, z));
                        const Vec3d gradNeg = m_backwardGrad(Vec3d(x, y, z));
                        //curvaturesPtr[i] = m_curvature(Vec3d(x, y, z));

                        // neg
                        gradientMagPtr[i * 2] =
                            std::pow(std::min(gradNeg[0], 0.0), 2) + std::pow(std::max(gradPos[0], 0.0), 2) +
                            std::pow(std::min(gradNeg[1], 0.0), 2) + std::pow(std::max(gradPos[1], 0.0), 2) +
                            std::pow(std::min(gradNeg[2], 0.0), 2) + std::pow(std::max(gradPos[2], 0.0), 2);

                        // pos
                        gradientMagPtr[i * 2 + 1] =
                            std::pow(std::max(gradNeg[0], 0.0), 2) + std::pow(std::min(gradPos[0], 0.0), 2) +
                            std::pow(std::max(gradNeg[1], 0.0), 2) + std::pow(std::min(gradPos[1], 0.0), 2) +
                            std::pow(std::max(gradNeg[2], 0.0), 2) + std::pow(std::min(gradPos[2], 0.0), 2);
                    }
                }
            });

        // Uniform advance
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
LevelSetModel::initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink)
{
    // Given no fields are interacting all quantities should be able to update in parallel
    for (size_t i = 0; i < m_evolveQuantitiesNodes.size(); i++)
    {
        m_taskGraph->addEdge(source, m_evolveQuantitiesNodes[i]);
        m_taskGraph->addEdge(m_evolveQuantitiesNodes[i], sink);
    }
}
}