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

#include "imstkRenderParticles.h"
#include "imstkLogger.h"

namespace imstk
{
RenderParticles::RenderParticles(const unsigned int maxNumParticles /*=128*/, const std::string& name /*= std::string("")*/)
    : Geometry(name)
{
    if (maxNumParticles <= 128)
    {
        m_maxNumParticles = maxNumParticles;
    }
    else
    {
        m_maxNumParticles = 128;
        LOG(WARNING) << "The maximum number of decals is 128";
    }

    m_vertexPositions[0] = Vec3d(0.5, 0.5, 0);
    m_vertexPositions[1] = Vec3d(0.5, -0.5, 0);
    m_vertexPositions[2] = Vec3d(-0.5, 0.5, 0);
    m_vertexPositions[3] = Vec3d(-0.5, -0.5, 0);

    m_vertexNormals[0] = Vec3d(0.0, 0.0, 1.0);
    m_vertexNormals[1] = Vec3d(0.0, 0.0, 1.0);
    m_vertexNormals[2] = Vec3d(0.0, 0.0, 1.0);
    m_vertexNormals[3] = Vec3d(0.0, 0.0, 1.0);

    m_vertexUVs[0] = Vec2d(1.0, 1.0);
    m_vertexUVs[1] = Vec2d(1.0, 0);
    m_vertexUVs[2] = Vec2d(0, 1.0);
    m_vertexUVs[3] = Vec2d(0, 0);

    m_triangles[0] = Vec3i(1, 0, 3);
    m_triangles[1] = Vec3i(0, 2, 3);
}

void
RenderParticles::setParticleSize(const float size)
{
    m_particleSize = size;
}

std::vector<std::unique_ptr<RenderParticle>>&
RenderParticles::getParticles()
{
    return m_particles;
}

void
RenderParticles::reset()
{
    m_numParticles = 0;
}

void
RenderParticles::incrementNumOfParticles()
{
    m_numParticles++;
}

unsigned int
RenderParticles::getNumParticles()
{
    return m_numParticles;
}

unsigned int
RenderParticles::getMaxNumParticles()
{
    return m_maxNumParticles;
}

void
RenderParticles::applyTransform(const Mat4d& imstkNotUsed(m))
{
    LOG(WARNING) << "applyTransform Not implemented!";
}
} // namespace imstk
