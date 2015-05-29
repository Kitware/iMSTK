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

#ifndef SMGEOMETRY_H
#define SMGEOMETRY_H

// SimMedTK includes
#include "smCore/smConfig.h"
#include "smUtilities/smVector.h"
#include "smUtilities/smMatrix.h"

#include "smRendering/smGLRenderer.h"
//#include "smRendering/smViewer.h"

//forward declaration
struct smSphere;

class smAnalyticalGeometry
{
public:
    smAnalyticalGeometry(){}
    ~smAnalyticalGeometry(){}

    virtual void translate(const smVec3d &t) = 0;
    virtual void rotate(const smMatrix33d &rot) = 0;

    virtual void draw()=0;

};

/// \brief  Simple Plane definition with unit normal and spatial location
class smPlane : public smAnalyticalGeometry
{
public:
    smPlane(){}
    ~smPlane(){}

    /// \brief create a plane with point and unit normal
    smPlane(const smVec3d &p, const smVec3d &n)
    {
        this->point = p;
        this->unitNormal = n;
        this->width = 1.0;
        
        this->drawPointsOrig[0] = smVec3d(width, 0, 0);
        this->drawPointsOrig[1] = smVec3d(0, width, 0);
        this->drawPointsOrig[2] = smVec3d(-width, 0, 0);
        this->drawPointsOrig[3] = smVec3d(0, -width, 0);

        this->movedOrRotated = true;
    }

    double distance(const smVec3d &p_vector)
    {
        auto m = (p_vector - this->point).dot(this->unitNormal);
        return m;
    };

    smVec3d project(const smVec3d &p_vector)
    {
        return p_vector - ((this->point - p_vector)*this->unitNormal.transpose())*this->unitNormal;
    };

    const smVec3d &getUnitNormal() const
    {
        return this->unitNormal;
    }

    void setModified(bool s)
    {
        this->movedOrRotated = s;
    };

    void setUnitNormal(const smVec3d &normal)
    {
        this->unitNormal = normal;

        this->movedOrRotated = true;
    }

    const smVec3d &getPoint() const
    {
        return this->point;
    }

    void setPoint(const smVec3d &p)
    {
        this->point = p;

        this->movedOrRotated = true;
    }

    void translate(const smVec3d &t)
    {
        this->point += t;

        this->movedOrRotated = true;
    }

    void rotate(const smMatrix33d &rot)
    {
        this->unitNormal = rot * this->unitNormal;

        this->movedOrRotated = true;
    }

    void setDrawPoint(const smVec3d &p1, const smVec3d &p2, const smVec3d &p3, const smVec3d &p4)
    {
        this->drawPointsOrig[0] = p1;
        this->drawPointsOrig[1] = p2;
        this->drawPointsOrig[2] = p3;
        this->drawPointsOrig[3] = p4;

        this->movedOrRotated = true;
    }

    void setWidth(double w)
    {
        this->width = w;
    }

    void updateDrawPoints()
    {
        smVec3d ny = smVec3d(0.0, unitNormal[2], -unitNormal[1]);
        smVec3d nz = ny.cross(unitNormal);
        ny.normalize();
        nz.normalize();

        smMatrix33d R;
        R << this->unitNormal[0], ny[1], nz[2],
             this->unitNormal[0], ny[1], nz[2],
             this->unitNormal[0], ny[1], nz[2];

        for (int i = 0; i < 4; i++)
        {
            this->drawPoints[i] = this->point + R*this->drawPointsOrig[i];
        }
        this->movedOrRotated = false;
    }

    void draw()
    {
        if (this->movedOrRotated)
        {
            updateDrawPoints();
        }
        glEnable(GL_LIGHTING);

        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, smColor::colorGray.toGLColor());
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, smColor::colorWhite.toGLColor());
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, smColor::colorGray.toGLColor());

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glPushMatrix();
            glBegin(GL_QUADS);
                glVertex3f(this->drawPoints[0][0], this->drawPoints[0][1], this->drawPoints[0][2]);
                glVertex3f(this->drawPoints[1][0], this->drawPoints[1][1], this->drawPoints[1][2]);
                glVertex3f(this->drawPoints[2][0], this->drawPoints[2][1], this->drawPoints[2][2]);
                glVertex3f(this->drawPoints[3][0], this->drawPoints[3][1], this->drawPoints[3][2]);
            glEnd();
        glPopMatrix();

        glDisable(GL_BLEND);

        glEnable(GL_LIGHTING);

    }

private:
    /// \brief unit normal of the plane
    smVec3d unitNormal;

    /// \brief any point on the plane
    smVec3d point;

    /// \brief true if the plane is static
    bool movedOrRotated;

    /// \brief width od the plane for rendering
    double width;

    /// \brief four points used to render plane
    smVec3d drawPoints[4];

    /// \brief four points used to render plane
    smVec3d drawPointsOrig[4];
};


/// \brief sphere with center and radius
class smSphere : public smAnalyticalGeometry
{
public:
    /// \brief constructor
    smSphere();

    /// \brief sphere constructor with center and radius
    smSphere(const smVec3d &c, const double &r)
    {
        this->center = c;
        this->radius = r;
    }

    ~smSphere(){}

    void setRadius(const double r)
    {
        this->radius = r;
    }

    void setCenter(const smVec3d& c)
    {
        this->center = c;
    }

    void incrementRadius(const double r)
    {
        this->radius += r;
    }

    void translate(const smVec3d &t)
    {
        center += t;
    }

    void rotate(const smMatrix33d &rot)
    {
        //Its a sphere! nothing to be done.
    }

    void draw()
    {
        // add sphere rendering here
    }

    double getRadius() const
    {
        return this->radius;
    }

    const smVec3d &getCenter() const
    {
        return this->center;
    }

private:
    /// \brief center of sphere
    smVec3d center;

    /// \brief radius of sshere
    double radius;
};

/// \brief cube
struct smCube
{
    /// \brief cube center
    smVec3d center;

    /// \brief cube length
    double sideLength;

    /// \brief constructor
    smCube();

    /// \brief subdivides the cube in mulitple cube with given number of cubes identified for each axis with p_divisionPerAxis
    void subDivide(smInt p_divisionPerAxis, smCube *p_cube);

    /// \brief expands the cube. increases the edge length with expansion*edge length
    void expand(double p_expansion);

    /// \brief returns the left most corner
    smVec3d leftMinCorner() const ;

    /// \brief returns right most corner
    smVec3d rightMaxCorner() const;

    /// \brief returns the smallest sphere encapsulates the cube
    smSphere getCircumscribedSphere();

    /// \brief returns the  sphere with half edge of the cube as a radius
    smSphere getInscribedSphere();

    /// \brief get tangent sphere
    smSphere getTangent2EdgeSphere();
};




/// \brief Axis Aligned bounding box declarions
class smAABB
{
public:
    /// \brief minimum x,y,z point
    smVec3d aabbMin;

    /// \brief maximum x,y,z point
    smVec3d aabbMax;

    const smVec3d &getMax() const
    {
        return aabbMax;
    }

    const smVec3d &getMin() const
    {
        return aabbMin;
    }

    /// \brief constrcutor. The default is set to origin for aabbMin and aabbMax
    smAABB();

    /// \brief center of the AABB
    smVec3d center() const;

    /// \brief check if two AABB overlaps
    static smBool checkOverlap(const smAABB &p_aabbA, const smAABB &p_aabbB);

    /// \brief set  p_aabb to the current one
    const smAABB &operator=(const smAABB &p_aabb);

    /// \brief scale the AABB
    smAABB &operator*(const double p_scale);

    /// \brief sub divides p_length will be used to create the slices
    void subDivide(const double p_length, const smInt p_divison, smAABB *p_aabb) const;

    /// \brief divides current AABB in x,y,z axes with specificed divisions. results are placed in p_aabb
    void subDivide(const smInt p_divisionX, const smInt p_divisionY, const smInt p_divisionZ, smAABB *p_aabb) const;

    /// \brief divides current AABB in all axes with specificed p_division. results are placed in p_aabb
    void subDivide(const smInt p_division, smAABB *p_aabb) const;

    /// \brief returns half of X edge of AABB
    double halfSizeX() const;

    /// \brief returns half of Y edge of AABB
    double halfSizeY() const;

    /// \brief returns half of Z edge of AABB
    double halfSizeZ() const;

    /// \brief expands aabb with p_factor
    void expand(const double &p_factor);

    void draw() const
    {
        glBegin(GL_LINES);
        {
            glVertex3d(aabbMin[0], aabbMin[1], aabbMin[2]);
            glVertex3d(aabbMin[0], aabbMin[1], aabbMax[2]);
            glVertex3d(aabbMin[0], aabbMax[1], aabbMin[2]);
            glVertex3d(aabbMin[0], aabbMax[1], aabbMax[2]);
            glVertex3d(aabbMax[0], aabbMin[1], aabbMin[2]);
            glVertex3d(aabbMax[0], aabbMin[1], aabbMax[2]);
            glVertex3d(aabbMax[0], aabbMax[1], aabbMin[2]);
            glVertex3d(aabbMax[0], aabbMax[1], aabbMax[2]);

            glVertex3d(aabbMin[0], aabbMin[1], aabbMin[2]);
            glVertex3d(aabbMin[0], aabbMax[1], aabbMin[2]);
            glVertex3d(aabbMin[0], aabbMin[1], aabbMax[2]);
            glVertex3d(aabbMin[0], aabbMax[1], aabbMax[2]);
            glVertex3d(aabbMax[0], aabbMin[1], aabbMin[2]);
            glVertex3d(aabbMax[0], aabbMax[1], aabbMin[2]);
            glVertex3d(aabbMax[0], aabbMin[1], aabbMax[2]);
            glVertex3d(aabbMax[0], aabbMax[1], aabbMax[2]);

            glVertex3d(aabbMin[0], aabbMin[1], aabbMin[2]);
            glVertex3d(aabbMax[0], aabbMin[1], aabbMin[2]);
            glVertex3d(aabbMin[0], aabbMax[1], aabbMin[2]);
            glVertex3d(aabbMax[0], aabbMax[1], aabbMin[2]);
            glVertex3d(aabbMin[0], aabbMin[1], aabbMax[2]);
            glVertex3d(aabbMax[0], aabbMin[1], aabbMax[2]);
            glVertex3d(aabbMin[0], aabbMax[1], aabbMax[2]);
            glVertex3d(aabbMax[0], aabbMax[1], aabbMax[2]);
        }
        glEnd();
    }

    void reset()
    {
        this->aabbMin << std::numeric_limits<float>::max(),
            std::numeric_limits<float>::max(),
            std::numeric_limits<float>::max();
        this->aabbMax << std::numeric_limits<float>::min(),
            std::numeric_limits<float>::min(),
            std::numeric_limits<float>::min();
    }

    void extend(const smAABB &other)
    {
        this->aabbMin = this->aabbMin.array().min(other.getMin().array());
        this->aabbMax = this->aabbMax.array().max(other.getMax().array());
    }
};

#endif
