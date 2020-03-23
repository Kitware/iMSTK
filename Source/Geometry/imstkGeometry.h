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

#include "g3log/g3log.hpp"

#include "imstkMath.h"
#include "imstkParallelUtils.h"

#include <string>
#include <memory>
#include <numeric>
#include <unordered_set>
#include <set>


namespace imstk
{
///
/// \class Geometry
/// \brief Base class for any geometrical representation
///
class Geometry
{
public:
    ///
    /// \brief Enumeration for the geometry type
    ///
    enum class Type
    {
        Plane,
        Sphere,
        Cylinder,
        Cube,
        Capsule,
        PointSet,
        SurfaceMesh,
        TetrahedralMesh,
        HexahedralMesh,
        LineMesh,
        Decal,
        DecalPool,
        RenderParticles,
        ImageData
    };

    ///
    /// \brief Enumeration for the transformation to apply
    /// \params ApplyToTransform to apply the transformation to the data
    /// \params ApplyToTransform to apply the transformation to the transform matrix
    ///
    enum class TransformType
    {
        ApplyToData,
        ConcatenateToTransform
    };

    ///
    /// \brief Enumeration for the data to retrieve
    /// \params PreTransform for data where transform matrix is not applied
    /// \params PostTransform for data where transform matrix is applied
    ///
    enum class DataType
    {
        PreTransform,
        PostTransform
    };

    ///
    /// \brief Constructor
    ///
    Geometry(const Type type, const std::string name = std::string(""));

    ///
    /// \brief Destructor
    ///
    virtual ~Geometry();

    ///
    /// \brief Print
    ///
    virtual void print() const;

    ///
    /// \brief Returns the volume of the geometry (if valid)
    ///
    virtual double getVolume() const = 0;

    ///
    /// \brief Compute the bounding box for the geometry
    ///
    virtual void computeBoundingBox(Vec3d& lowerCorner, Vec3d& upperCorner, const double paddingPercent = 0.0);

    ///
    /// \brief Translate the geometry in Cartesian space
    ///
    void translate(const Vec3d& t, TransformType type = TransformType::ConcatenateToTransform);
    void translate(double x, double y, double z, TransformType type = TransformType::ConcatenateToTransform);

    ///
    /// \brief Rotate the geometry in Cartesian space
    ///
    void rotate(const Quatd& q, TransformType type = TransformType::ConcatenateToTransform);
    void rotate(const Mat3d& m, TransformType type = TransformType::ConcatenateToTransform);
    void rotate(const Vec3d& axis, double angle, TransformType type = TransformType::ConcatenateToTransform);

    ///
    /// \brief Scale in Cartesian directions
    ///
    void scale(double scaling, TransformType type = TransformType::ConcatenateToTransform);

    ///
    /// \brief Applies a rigid transform to the geometry
    ///
    void transform(RigidTransform3d T, TransformType type = TransformType::ConcatenateToTransform);

    ///
    /// \brief Get/Set translation
    ///
    Vec3d getTranslation() const;
    void setTranslation(const Vec3d t);
    void setTranslation(double x, double y, double z);

    ///
    /// \brief Get/Set rotation
    ///
    Mat3d getRotation() const;
    void setRotation(const Mat3d m);
    void setRotation(const Quatd q);
    void setRotation(const Vec3d axis, const double angle);

    ///
    /// \brief Get/Set scaling
    ///
    double getScaling() const;
    void setScaling(const double s);

    ///
    /// \brief Returns the type of the geometry
    ///
    Type getType() const;

    ///
    /// \brief Get name of the geometry
    ///
    const std::string& getName() const { return m_name; }

    ///
    /// \brief Returns the string representing the type name of the geometry
    ///
    const std::string getTypeName() const;

    ///
    /// \brief Returns true if the geometry is a mesh, else returns false
    ///
    bool isMesh() const;

    ///
    /// \brief Get the global (unique) index of the geometry
    ///
    uint32_t getGlobalIndex() const { return m_geometryIndex; }

    ///
    /// \brief Get a pointer to geometry that has been registered globally
    ///
    static uint32_t getTotalNumberGeometries() { return s_NumGeneratedGegometries; }

protected:
    ///
    /// \brief Get a unique ID for the new generated geometry object
    ///
    static uint32_t getUniqueID();

    /// Mutex lock for thread-safe counter update and name set update
    static ParallelUtils::SpinLock s_GeomGlobalLock;

    /// Total number of geometries that have been created in this program
    static uint32_t s_NumGeneratedGegometries;

    /// Set of string names of all generated geometries, used to check for name duplication
    static tbb::concurrent_unordered_set<std::string> s_sGegometryNames;

    friend class VTKRenderer;
    friend class VTKRenderDelegate;

    friend class VulkanLineMeshRenderDelegate;
    friend class VulkanRenderDelegate;
    friend class VulkanSurfaceMeshRenderDelegate;
    friend class VulkanRenderer;

    virtual void applyTranslation(const Vec3d t) = 0;
    virtual void applyRotation(const Mat3d r)    = 0;
    virtual void applyScaling(const double s)    = 0;
    virtual void updatePostTransformData() const = 0;

    Type m_type;                 ///> Type of geometry
    std::string m_name;          ///> Unique name for each geometry
    uint32_t    m_geometryIndex; ///> Unique ID assigned to each geometry upon construction

    bool m_dataModified      = false;
    bool m_transformModified = false;
    mutable bool m_transformApplied = true;

    RigidTransform3d m_transform = RigidTransform3d::Identity(); ///> Transformation matrix
    double m_scaling = 1.0;
};

class GeometricUtils {
public:
    ///
    /// \brief Reverse Cuthill-Mckee; return the permutation vector that maps from new indices to old indices
    //
    /// \param conn element-to-vertex connectivity
    /// \param numVerts number of vertices
    ///
    template <typename ElemConn>
    static std::vector<size_t>
    RCM(const std::vector<ElemConn>& conn, const size_t numVerts)
    {
        // connectivity of vertex-to-vertex
        std::vector<std::unordered_set<size_t>> vertToVert;
        buildVertToVert(conn, numVerts, vertToVert);
        return RCM(vertToVert);
    }

    ///
    /// \brief Reverse Cuthill-Mckee; returns the permutation vector that map from new indices to old indices
    ///
    /// \param neighbors array of neighbors of each vertex; eg, neighbors[i] is an object containing all neighbors of vertex-i
    ///
    template <typename NBR>
    static std::vector<size_t>
    RCM(const std::vector<NBR>& neighbors)
    {
        const size_t INVALID = std::numeric_limits<size_t>::max();

        // is greater in terms of degrees
        auto isGreater = [&neighbors](const size_t i, const size_t j) {
            return neighbors[i].size() > neighbors[j].size() ? true : false;
        };

        const size_t numVerts = neighbors.size();

        std::vector<size_t> P(numVerts);
        for (size_t i = 0; i < numVerts; ++i)
        {
            P[i] = i;
        }
        std::sort(P.begin(), P.end(), isGreater);

        // an alternative is to use std::set for P
        // std::set<size_t, isGreater> P;
        // for (size_t i=0; i<numVerts; ++i)
        // {
        //     P.insert(i);
        // }

        std::vector<size_t> R(numVerts, INVALID);   // permutation
        std::queue<size_t>  Q;                      // queue
        std::vector<bool>   isInP(numVerts, true);  // if a vertex is still in P
        size_t              pos = 0;  // track how many vertices are put into R

        ///
        /// \brief Move a vertex into R, and enque its neighbors into Q in ascending order.
        /// \param vid index of vertex to be moved into R
        ///
        auto moveIntoRAndNbrIntoQ = [&neighbors, &isInP, &pos, &R, &Q](const size_t vid) {
            R[pos] = vid;
            ++pos;
            isInP[vid] = false;

            // Put the unordered neighbors into Q, in ascending order.
            // first find unordered neighbors
            std::set<size_t> unorderedNbrs;
            for (auto nbr : neighbors[vid])
            {
                if (isInP[nbr])
                {
                    unorderedNbrs.insert(nbr);
                }
            }

            for (auto nbr : unorderedNbrs)
            {
                Q.push(nbr);
                isInP[nbr] = false;
            }
        };

        size_t pCur = 0;

        ///
        /// \brief pop a vertex that is not ordered from \p P
        ///
        auto popFromP = [&pCur, &isInP]() {
            for (size_t vid = pCur; vid < isInP.size(); ++vid)
            {
                if (isInP[vid])
                {
                    isInP[vid] = false;
                    pCur       = vid;
                    return vid;
                }
            }
            return INVALID;
        };

        // main loop
        while (true)
        {
            std::size_t parent = popFromP();
            if (parent == INVALID)
            {
                break;
            }

            moveIntoRAndNbrIntoQ(parent);

            while (!Q.empty())
            {
                parent = Q.front();
                Q.pop();
                moveIntoRAndNbrIntoQ(parent);
            }

            // here we have empty Q
        }

        CHECK(pos == numVerts);

        std::reverse(R.begin(), R.end());
        return R;
    }

private:
    ///
    /// \brief Build the vertex-to-vertex connectivity
    /// 
    /// \param conn element-to-vertex connectivity
    /// \param numVerts number of vertices
    /// \param vertToVert the vertex-to-vertex connectivity on return
    ///
    template <typename ElemConn>
    static void
    buildVertToVert(const std::vector<ElemConn>&             conn,
                    const size_t                             numVerts,
                    std::vector<std::unordered_set<size_t>>& vertToVert)
    {
        // constexpr size_t numVertPerElem = ElemConn::size();
        std::vector<size_t> vertToElemPtr(numVerts + 1, 0);
        std::vector<size_t> vertToElem;

        // find the number of adjacent elements for each vertex
        for (const auto& vertices : conn)
        {
            for (auto vid : vertices)
            {
                vertToElemPtr[vid + 1] += 1;
            }
        }

        // accumulate pointer
        for (size_t i = 0; i < numVerts; ++i)
        {
            vertToElemPtr[i + 1] += vertToElemPtr[i];
        }

        // track the number
        auto   pos    = vertToElemPtr;
        size_t totNum = vertToElemPtr.back();

        vertToElem.resize(totNum);

        for (size_t eid = 0; eid < conn.size(); ++eid)
        {
            for (auto vid : conn[eid])
            {
                vertToElem[pos[vid]] = eid;
                ++pos[vid];
            }
        }

        // connectivity of vertex-to-vertex
        vertToVert.resize(numVerts);
        auto getVertexNbrs = [&vertToElem, &vertToElemPtr, &conn, &vertToVert](const size_t i) {
            const auto ptr0 = vertToElemPtr[i];
            const auto ptr1 = vertToElemPtr[i + 1];
            size_t     eid;

            for (auto ptr = ptr0; ptr < ptr1; ++ptr)
            {
                eid = vertToElem[ptr];
                for (auto vid : conn[eid])
                {
                    // vertex-i itself is also included.
                    vertToVert[i].insert(vid);
                }
            }
        };

        for (size_t i = 0; i < numVerts; ++i)
        {
            getVertexNbrs(i);
        }
    }

};  // class GeometricUtils 


} //imstk
