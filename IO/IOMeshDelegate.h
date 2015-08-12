// This file is part of the SimMedTK project.
// Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
//                        Rensselaer Polytechnic Institute
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//---------------------------------------------------------------------------
//
// Authors:
//
// Contact:
//---------------------------------------------------------------------------

#ifndef IOMESHDELEGATE_H
#define IOMESHDELEGATE_H

#include <cstdint>
#include <memory>

#include "IO/IOMesh.h"
#include "Core/Config.h"
#include "Mesh/SurfaceMesh.h"
#include "Mesh/VegaVolumetricMesh.h"

// VegaFEM includes
#include "tetMesh.h"
#include "cubicMesh.h"
#include "generateSurfaceMesh.h"
#include "objMesh.h"

class IOMeshDelegate
{
public:
    typedef std::shared_ptr<IOMeshDelegate> Ptr;

public:
    void setSource(IOMesh *src)
    {
        meshIO = src;
    }

    virtual void read() { }
    virtual void write(){ }

protected:
    void setSurfaceMesh(const std::vector<core::Vec3d> &vertices,
                        const std::vector<std::array<size_t,3>> &triangleArray)
    {
        auto mesh = std::make_shared<SurfaceMesh>();
        mesh->setVertices(vertices);
        mesh->setTriangles(triangleArray);
        this->meshIO->setMesh(mesh);
    }

    void setVegaTetraMesh(const std::vector<core::Vec3d> &vertices,
                          const std::vector<std::array<size_t,4>> &tetraArray,
                          const std::vector<size_t> &bdConditions,
                          const core::Vec3d &material)
    {
        auto tetraMesh = std::make_shared<VegaVolumetricMesh>();
        std::vector<int> vegaElements;

        std::for_each(tetraArray.begin(),tetraArray.end(),
                      [&](const std::array<size_t,4> &i)
                      {
                          vegaElements.push_back(i[0]); vegaElements.push_back(i[1]);
                          vegaElements.push_back(i[2]); vegaElements.push_back(i[3]);
                      });

        double *v = const_cast<double*>(vertices.data()->data());
        int *e = vegaElements.data();

        int v_size = vertices.size();
        int e_size = tetraArray.size();
        auto vegaMesh = std::make_shared<TetMesh>(v_size,
                                                  v,
                                                  e_size,
                                                  e,
                                                  material[0],
                                                  material[1],
                                                  material[2]);
        tetraMesh->setVegaMesh(vegaMesh);
        this->meshIO->setMesh(tetraMesh);
    }

    void setVegaTetraMesh(const std::vector<core::Vec3d> &vertices,
                          const std::vector<std::array<size_t,4>> &tetraArray)
    {
        auto tetraMesh = std::make_shared<VegaVolumetricMesh>();
        std::vector<int> vegaElements;

        std::for_each(tetraArray.begin(),tetraArray.end(),
                      [&](const std::array<size_t,4> &i)
                      {
                          vegaElements.push_back(i[0]); vegaElements.push_back(i[1]);
                          vegaElements.push_back(i[2]); vegaElements.push_back(i[3]);
                      });

        double *v = const_cast<double*>(vertices.data()->data());
        int *e = vegaElements.data();
        int v_size = vertices.size();
        int e_size = tetraArray.size();
        auto vegaMesh = std::make_shared<TetMesh>(v_size,
                                                  v,
                                                  e_size,
                                                  e);
        tetraMesh->setVegaMesh(vegaMesh);
        this->meshIO->setMesh(tetraMesh);
    }

    void setVegaHexaMesh(const std::vector<core::Vec3d> &vertices,
                         const std::vector<std::array<size_t,8>> &hexaArray,
                         const std::vector<size_t> &bdConditions,
                         const std::array<double,3> &material)
    {
        auto hexaMesh = std::make_shared<VegaVolumetricMesh>();
        std::vector<int> vegaElements;

        std::for_each(hexaArray.begin(),hexaArray.end(),
                      [&](const std::array<size_t,8> &i)
                      {
                          vegaElements.push_back(i[0]); vegaElements.push_back(i[1]);
                          vegaElements.push_back(i[2]); vegaElements.push_back(i[3]);
                          vegaElements.push_back(i[4]); vegaElements.push_back(i[5]);
                          vegaElements.push_back(i[6]); vegaElements.push_back(i[7]);
                      });

        double *v = const_cast<double*>(vertices.data()->data());
        int *e = vegaElements.data();
        int v_size = vertices.size();
        int e_size = hexaArray.size();
        auto vegaMesh = std::make_shared<CubicMesh>(v_size,
                                                    v,
                                                    e_size,
                                                    e,
                                                    material[0],
                                                    material[1],
                                                    material[2]);
        hexaMesh->setVegaMesh(vegaMesh);
        this->meshIO->setMesh(hexaMesh);
    }

    void setVegaHexaMesh(const std::vector<core::Vec3d> &vertices,
                         const std::vector<std::array<size_t,8>> &hexaArray)
    {
        auto hexaMesh = std::make_shared<VegaVolumetricMesh>();
        std::vector<int> vegaElements;

        std::for_each(hexaArray.begin(),hexaArray.end(),
                      [&](const std::array<size_t,8> &i)
                      {
                          vegaElements.push_back(i[0]); vegaElements.push_back(i[1]);
                          vegaElements.push_back(i[2]); vegaElements.push_back(i[3]);
                          vegaElements.push_back(i[4]); vegaElements.push_back(i[5]);
                          vegaElements.push_back(i[6]); vegaElements.push_back(i[7]);
                      });

        double *v = const_cast<double*>(vertices.data()->data());
        int *e = vegaElements.data();
        int v_size = vertices.size();
        int e_size = hexaArray.size();
        auto vegaMesh = std::make_shared<CubicMesh>(v_size,
                                                    v,
                                                    e_size,
                                                    e);
        hexaMesh->setVegaMesh(vegaMesh);
        this->meshIO->setMesh(hexaMesh);
    }

    void reorderSurfaceTopology(std::vector<std::array<size_t,3>> &triangleArray,
                                const std::vector<core::Vec3d> &vertices,
                                std::vector<core::Vec3d> &surfaceVertices,
                                std::unordered_map<size_t,size_t> &uniqueVertexArray
                               )
    {
        auto CompareTriangles = [&](const std::array<size_t,3> &x,
                                    const std::array<size_t,3> &y) -> bool
        {
            return (x[0] < y[0])
                || ((x[0] == y[0]) && (x[1] < y[1]))
                || ((x[1] == y[1]) && (x[2] < y[2])) ? true: false;
        };

        std::sort(std::begin(triangleArray),std::end(triangleArray),CompareTriangles);

        std::set<size_t> uniqueVertexSet;
        for(const auto &t : triangleArray)
        {
            for(const auto &v : t)
            {
                uniqueVertexSet.insert(v);
            }
        }
        size_t index = 0;
        for(const auto &v : uniqueVertexSet)
        {
            uniqueVertexArray[index++] = v;
        }
        uniqueVertexSet.clear();
        for(auto &t : triangleArray)
        {
            for(auto &v : t)
            {
                v = uniqueVertexArray[v];
            }
        }

        for(const auto &v : uniqueVertexSet)
        {
            surfaceVertices.emplace_back(vertices[v]);
        }
    }

protected:
    IOMesh *meshIO;
};
#endif // READERDELEGATE_H
