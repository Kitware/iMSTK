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

#ifndef SM_ANALYTICAL_GEOMETRY_H
#define SM_ANALYTICAL_GEOMETRY_H

#include "Core/Geometry.h"

//forward declaration
struct Sphere;

class AnalyticalGeometry : public GeometryBase
{
public:
	///
	/// \brief Constructor
	///
    AnalyticalGeometry(){}

	///
	/// \brief Destructor
	///
    ~AnalyticalGeometry(){}
};

///
/// \brief  Analytical plane defined by unit normal and spatial location
///
class Plane : public AnalyticalGeometry
{
public:
    ///
    /// \brief Constructor
    ///
    Plane()
      {
      this->width = 100.0;

      this->setRenderDelegate(
        Factory<RenderDelegate>::createSubclassForGroup(
            "RenderDelegate", RenderDelegate::RendererType::VTK));
      }

    ///
    /// \brief Destructor
    ///
    ~Plane(){}

    ///
    /// \brief Create a plane with point and unit normal
    ///
    Plane(const core::Vec3d &p, const core::Vec3d &n)
    {
        this->point = p;
        this->unitNormal = n.normalized();
        this->width = 100.0;

        this->drawPointsOrig[0] = core::Vec3d(width, 0, 0);
        this->drawPointsOrig[1] = core::Vec3d(0, width, 0);
        this->drawPointsOrig[2] = core::Vec3d(-width, 0, 0);
        this->drawPointsOrig[3] = core::Vec3d(0, -width, 0);

        this->movedOrRotated = true;

        this->setRenderDelegate(
          Factory<RenderDelegate>::createSubclass(
            "RenderDelegate", "PlaneRenderDelegate"));
    }

    ///
    /// \brief Compute and return the distance to a given point
    ///
    double distance(const core::Vec3d &p_vector)
    {
        auto m = (p_vector - this->point).dot(this->unitNormal);
        return m;
    };

    ///
    /// \brief Returns the projection of a given point on the plane
    ///
    core::Vec3d project(const core::Vec3d &p_vector)
    {
        return p_vector -
            ((this->point - p_vector)*this->unitNormal.transpose())*this->unitNormal;
    };

    ///
    /// \brief Return the normal of the plane
    ///
    const core::Vec3d &getUnitNormal() const
    {
        return this->unitNormal;
    }

    ///
    /// \brief
    ///
    void setModified(bool s)
    {
        this->movedOrRotated = s;
    };

    ///
    /// \brief
    ///
    void setUnitNormal(const core::Vec3d &normal)
    {
        this->unitNormal = normal;

        this->movedOrRotated = true;
    }

    ///
    /// \brief
    ///
    const core::Vec3d &getPoint() const
    {
        return this->point;
    }

    ///
    /// \brief
    ///
    void setPoint(const core::Vec3d &p)
    {
        this->point = p;

        this->movedOrRotated = true;
    }

    ///
    /// \brief
    ///
    void translate(const core::Vec3d &t)
    {
        this->point += t;

        this->movedOrRotated = true;
    }

    ///
    /// \brief
    ///
    void rotate(const core::Matrix33d &rot)
    {
        this->unitNormal = rot * this->unitNormal;

        this->movedOrRotated = true;
    }

    ///
    /// \brief
    ///
    void setDrawPoint(
        const core::Vec3d &p1,
        const core::Vec3d &p2,
        const core::Vec3d &p3,
        const core::Vec3d &p4)
    {
        this->drawPointsOrig[0] = p1;
        this->drawPointsOrig[1] = p2;
        this->drawPointsOrig[2] = p3;
        this->drawPointsOrig[3] = p4;

        this->movedOrRotated = true;
    }

    ///
    /// \brief
    ///
    double getWidth() const
    {
        return this->width;
    }

    ///
    /// \brief
    ///
    void setWidth(double w)
    {
        this->width = w;
    }

    ///
    /// \brief
    ///
    void updateDrawPoints()
    {
        core::Vec3d ny = core::Vec3d(0.0, unitNormal[2], -unitNormal[1]);
        core::Vec3d nz = ny.cross(unitNormal);
        ny.normalize();
        nz.normalize();

        core::Matrix33d R;
        R << this->unitNormal[0], ny[1], nz[2],
             this->unitNormal[0], ny[1], nz[2],
             this->unitNormal[0], ny[1], nz[2];

        for (int i = 0; i < 4; i++)
        {
            this->drawPoints[i] = this->point + R*this->drawPointsOrig[i];
        }
        this->movedOrRotated = false;
    }

private:
    ///
    /// \brief unit normal of the plane
    ///
    core::Vec3d unitNormal;

    ///
    /// \brief any point on the plane
    ///
    core::Vec3d point;

    ///
    /// \brief true if the plane is static
    ///
    bool movedOrRotated;

    ///
    /// \brief width of the plane for rendering
    ///
    double width;

    ///
    /// \brief four points used to render plane
    ///
    core::Vec3d drawPoints[4];

    ///
    /// \brief four points used to render plane
    ///
    core::Vec3d drawPointsOrig[4];
};

///
/// \brief Analytical sphere defined by a center and radius
///
class Sphere : public AnalyticalGeometry
{
public:
    ///
    /// \brief constructor
    ///
    Sphere();

    ///
    /// \brief sphere constructor with center and radius
    ///
    Sphere(const core::Vec3d &c, const double &r)
    {
        this->center = c;
        this->radius = r;
    }

    ///
    /// \brief
    ///
    ~Sphere(){}

    ///
    /// \brief
    ///
    void setRadius(const double r)
    {
        this->radius = r;
    }

    ///
    /// \brief
    ///
    void setCenter(const core::Vec3d& c)
    {
        this->center = c;
    }

    ///
    /// \brief
    ///
    void incrementRadius(const double r)
    {
        this->radius += r;
    }

    ///
    /// \brief
    ///
    void translate(const core::Vec3d &t)
    {
        center += t;
    }

    ///
    /// \brief
    ///
    void rotate(const core::Matrix33d &/*rot*/)
    {
        //Its a sphere! nothing to be done.
    }

    ///
    /// \brief
    ///
    double getRadius() const
    {
        return this->radius;
    }

    ///
    /// \brief
    ///
    const core::Vec3d &getCenter() const
    {
        return this->center;
    }

private:
    ///
    /// \brief center of sphere
    ///
    core::Vec3d center;

    ///
    /// \brief radius of sphere
    ///
    double radius;
};

///
/// \brief Cube defined by a center and side length
///
/// \todo finish the full class implementation
///
class Cube : public AnalyticalGeometry
{
public:
    ///
    /// \brief constructor
    ///
    Cube();

    ///
    /// \brief constructor
    ///
    ~Cube();

    ///
    /// \brief subdivides the cube in multiple cube with given number of cubes
    /// identified for each axis with p_divisionPerAxis
    ///
    void subDivide(int p_divisionPerAxis, Cube *p_cube);

    ///
    /// \brief expands the cube. increases the edge length with expansion*edge length
    ///
    void expand(double p_expansion);

    ///
    /// \brief returns the left most corner
    ///
    core::Vec3d leftMinCorner() const;

    ///
    /// \brief returns right most corner
    ///
    core::Vec3d rightMaxCorner() const;

    ///
    /// \brief returns the smallest sphere encapsulates the cube
    ///
    Sphere getCircumscribedSphere();

    ///
    /// \brief returns the  sphere with half edge of the cube as a radius
    ///
    Sphere getInscribedSphere();

    ///
    /// \brief get tangent sphere
    ///
    Sphere getTangent2EdgeSphere();

private:
    core::Vec3d center;///> cube center

    double sideLength;///> cube's side length
};

///
/// \brief Axis Aligned bounding box declarations
///
class AABB : public VisualArtifact
{
public:
    ///
    /// \brief minimum x,y,z point
    ///
    core::Vec3d aabbMin;

    ///
    /// \brief maximum x,y,z point
    ///
    core::Vec3d aabbMax;

    ///
    /// \brief
    ///
    const core::Vec3d &getMax() const
    {
        return aabbMax;
    }

    ///
    /// \brief
    ///
    const core::Vec3d &getMin() const
    {
        return aabbMin;
    }

    ///
    /// \brief constructor. The default is set to origin for aabbMin and aabbMax
    ///
    AABB();

    ///
    /// \brief center of the AABB
    ///
    core::Vec3d center() const;

    ///
    /// \brief check if two AABB overlaps
    ///
    static bool checkOverlap(const AABB &p_aabbA, const AABB &p_aabbB);

    ///
    /// \brief check if two AABB overlaps
    ///
    bool overlaps(const AABB &other) const;

    ///
    /// \brief set  p_aabb to the current one
    ///
    const AABB &operator=(const AABB &p_aabb);

    ///
    /// \brief scale the AABB
    ///
    AABB &operator*(const double p_scale);

    ///
    /// \brief sub divides p_length will be used to create the slices
    ///
    void subDivide(
        const double p_length,
        const int p_divison,
        AABB *p_aabb) const;

    ///
    /// \brief divides current AABB in x,y,z axes with specified divisions.
    /// results are placed in p_aabb
    ///
    void subDivide(
        const int p_divisionX,
        const int p_divisionY,
        const int p_divisionZ,
        AABB *p_aabb) const;

    ///
    /// \brief divides current AABB in all axes with specified p_division.
    /// results are placed in p_aabb
    ///
    void subDivide(const int p_division, AABB *p_aabb) const;

    ///
    /// \brief returns half of X edge of AABB
    ///
    double halfSizeX() const;

    ///
    /// \brief returns half of Y edge of AABB
    ///
    double halfSizeY() const;

    ///
    /// \brief returns half of Z edge of AABB
    ///
    double halfSizeZ() const;

    ///
    /// \brief expands aabb with p_factor
    ///
    void expand(const double &p_factor);

    ///
    /// \brief
    ///
    void reset()
    {
        this->aabbMin << std::numeric_limits<float>::max(),
            std::numeric_limits<float>::max(),
            std::numeric_limits<float>::max();
        this->aabbMax << std::numeric_limits<float>::min(),
            std::numeric_limits<float>::min(),
            std::numeric_limits<float>::min();
    }

    ///
    /// \brief
    ///
    void extend(const AABB &other)
    {
        this->aabbMin = this->aabbMin.array().min(other.getMin().array());
        this->aabbMax = this->aabbMax.array().max(other.getMax().array());
    }
};

#endif //SM_ANALYTICAL_GEOMETRY_H