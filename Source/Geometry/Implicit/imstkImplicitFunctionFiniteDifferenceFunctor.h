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

#include "imstkImplicitGeometry.h"
#include "imstkSignedDistanceField.h"

namespace imstk
{
///
/// \brief Base struct for gradient functors
///
struct ImplicitFunctionGradient
{
    public:
        virtual Vec3d operator()(const Vec3d& pos) const = 0;

        virtual void setDx(const Vec3d& dx)
        {
            this->m_dx    = dx;
            this->m_invDx = Vec3d(1.0 / dx[0], 1.0 / dx[1], 1.0 / dx[2]);
        }

        const Vec3d& getDx() const { return m_dx; }

        void setFunction(std::shared_ptr<ImplicitGeometry> func) { this->m_func = func; }

    protected:
        std::shared_ptr<ImplicitGeometry> m_func;
        Vec3d m_dx    = Vec3d(1.0, 1.0, 1.0);
        Vec3d m_invDx = Vec3d(1.0, 1.0, 1.0);
};

///
/// \brief Gradient given by central finite differences
///
struct ImplicitFunctionCentralGradient : public ImplicitFunctionGradient
{
    public:
        Vec3d operator()(const Vec3d& pos) const override
        {
            const ImplicitGeometry& funcRef = *m_func;
            return Vec3d(
            funcRef.getFunctionValue(Vec3d(pos[0] + m_dx[0], pos[1], pos[2])) - funcRef.getFunctionValue(Vec3d(pos[0] - m_dx[0], pos[1], pos[2])),
            funcRef.getFunctionValue(Vec3d(pos[0], pos[1] + m_dx[1], pos[2])) - funcRef.getFunctionValue(Vec3d(pos[0], pos[1] - m_dx[1], pos[2])),
            funcRef.getFunctionValue(Vec3d(pos[0], pos[1], pos[2] + m_dx[2])) - funcRef.getFunctionValue(Vec3d(pos[0], pos[1], pos[2] - m_dx[2]))).cwiseProduct(m_invDx) * 0.5;
        }
};

///
/// \brief Gradient given by central finite differences
///
struct StructuredCentralGradient : public ImplicitFunctionGradient
{
    public:
        Vec3d operator()(const Vec3d& pos) const override
        {
            const SignedDistanceField& funcRef = *static_cast<SignedDistanceField*>(m_func.get());
            return Vec3d(
            funcRef.getFunctionValueCoord(Vec3i(static_cast<int>(pos[0] + m_dxi[0]), static_cast<int>(pos[1]), static_cast<int>(pos[2]))) -
            funcRef.getFunctionValueCoord(Vec3i(static_cast<int>(pos[0] - m_dxi[0]), static_cast<int>(pos[1]), static_cast<int>(pos[2]))),
            funcRef.getFunctionValueCoord(Vec3i(static_cast<int>(pos[0]), static_cast<int>(pos[1] + m_dxi[1]), static_cast<int>(pos[2]))) -
            funcRef.getFunctionValueCoord(Vec3i(static_cast<int>(pos[0]), static_cast<int>(pos[1] - m_dxi[1]), static_cast<int>(pos[2]))),
            funcRef.getFunctionValueCoord(Vec3i(static_cast<int>(pos[0]), static_cast<int>(pos[1]), static_cast<int>(pos[2] + m_dxi[2]))) -
            funcRef.getFunctionValueCoord(Vec3i(static_cast<int>(pos[0]), static_cast<int>(pos[1]), static_cast<int>(pos[2] - m_dxi[2])))).cwiseProduct(m_invDx) * 0.5;
        }

        void setDx(const Vec3i& dx, const Vec3d& dxs)
        {
            m_dxi = dx;
            ImplicitFunctionGradient::setDx(dxs);
        }

    protected:
        using ImplicitFunctionGradient::setDx;
        Vec3i m_dxi;
};

///
/// \brief Gradient given by forward finite differences
///
struct ImplicitFunctionForwardGradient : public ImplicitFunctionGradient
{
    public:
        Vec3d operator()(const Vec3d& pos) const override
        {
            const ImplicitGeometry& funcRef      = *m_func;
            const double            centralValue = funcRef.getFunctionValue(Vec3d(pos[0], pos[1], pos[2]));
            const double            maxValueX    = funcRef.getFunctionValue(Vec3d(pos[0] + m_dx[0], pos[1], pos[2]));
            const double            maxValueY    = funcRef.getFunctionValue(Vec3d(pos[0], pos[1] + m_dx[1], pos[2]));
            const double            maxValueZ    = funcRef.getFunctionValue(Vec3d(pos[0], pos[1], pos[2] + m_dx[2]));
            return Vec3d(
        maxValueX - centralValue,
        maxValueY - centralValue,
        maxValueZ - centralValue).cwiseProduct(m_invDx);
        }
};

///
/// \brief Gradient given by backward finite differences
///
struct ImplicitFunctionBackwardGradient : public ImplicitFunctionGradient
{
    public:
        Vec3d operator()(const Vec3d& pos) const override
        {
            const ImplicitGeometry& funcRef      = *m_func;
            const double            centralValue = funcRef.getFunctionValue(Vec3d(pos[0], pos[1], pos[2]));
            const double            minValueX    = funcRef.getFunctionValue(Vec3d(pos[0] - m_dx[0], pos[1], pos[2]));
            const double            minValueY    = funcRef.getFunctionValue(Vec3d(pos[0], pos[1] - m_dx[1], pos[2]));
            const double            minValueZ    = funcRef.getFunctionValue(Vec3d(pos[0], pos[1], pos[2] - m_dx[2]));
            return Vec3d(
        centralValue - minValueX,
        centralValue - minValueY,
        centralValue - minValueZ).cwiseProduct(m_invDx);
        }
};

///
/// \brief Gradient given by forward finite differences
///
struct StructuredForwardGradient : public ImplicitFunctionGradient
{
    public:
        inline Vec3d operator()(const Vec3d& pos) const override
        {
            const SignedDistanceField& funcRef      = *static_cast<SignedDistanceField*>(m_func.get());
            const double               centralValue = funcRef.getFunctionValueCoord(Vec3i(static_cast<int>(pos[0]), static_cast<int>(pos[1]), static_cast<int>(pos[2])));
            const double               maxValueX    = funcRef.getFunctionValueCoord(Vec3i(static_cast<int>(pos[0] + m_dxi[0]), static_cast<int>(pos[1]), static_cast<int>(pos[2])));
            const double               maxValueY    = funcRef.getFunctionValueCoord(Vec3i(static_cast<int>(pos[0]), static_cast<int>(pos[1] + m_dxi[1]), static_cast<int>(pos[2])));
            const double               maxValueZ    = funcRef.getFunctionValueCoord(Vec3i(static_cast<int>(pos[0]), static_cast<int>(pos[1]), static_cast<int>(pos[2] + m_dxi[2])));
            return Vec3d(
        maxValueX - centralValue,
        maxValueY - centralValue,
        maxValueZ - centralValue).cwiseProduct(m_invDx);
        }

        void setDx(const Vec3i& dx, const Vec3d& dxs)
        {
            m_dxi = dx;
            ImplicitFunctionGradient::setDx(dxs.cwiseProduct(m_dxi.cast<double>()));
        }

    protected:
        using ImplicitFunctionGradient::setDx;
        Vec3i m_dxi;
};

///
/// \brief Gradient given by backward finite differences
///
struct StructuredBackwardGradient : public ImplicitFunctionGradient
{
    public:
        inline Vec3d operator()(const Vec3d& pos) const override
        {
            const SignedDistanceField& funcRef      = *static_cast<SignedDistanceField*>(m_func.get());
            const double               centralValue = funcRef.getFunctionValueCoord(Vec3i(static_cast<int>(pos[0]), static_cast<int>(pos[1]), static_cast<int>(pos[2])));
            const double               minValueX    = funcRef.getFunctionValueCoord(Vec3i(static_cast<int>(pos[0] - m_dxi[0]), static_cast<int>(pos[1]), static_cast<int>(pos[2])));
            const double               minValueY    = funcRef.getFunctionValueCoord(Vec3i(static_cast<int>(pos[0]), static_cast<int>(pos[1] - m_dxi[1]), static_cast<int>(pos[2])));
            const double               minValueZ    = funcRef.getFunctionValueCoord(Vec3i(static_cast<int>(pos[0]), static_cast<int>(pos[1]), static_cast<int>(pos[2] - m_dxi[2])));
            return Vec3d(
        centralValue - minValueX,
        centralValue - minValueY,
        centralValue - minValueZ).cwiseProduct(m_invDx);
        }

        void setDx(const Vec3i& dx, const Vec3d& dxs)
        {
            m_dxi = dx;
            ImplicitFunctionGradient::setDx(dxs);
        }

    protected:
        using ImplicitFunctionGradient::setDx;
        Vec3i m_dxi;
};

///
/// \brief Curvature given in structured coordinates
///
struct ImplicitStructuredCurvature
{
    public:
        double operator()(const Vec3d& pos) const
        {
            const SignedDistanceField& funcRef = *static_cast<SignedDistanceField*>(m_func.get());

            const double central = funcRef.getFunctionValueCoord(Vec3i(static_cast<int>(pos[0]), static_cast<int>(pos[1]), static_cast<int>(pos[2])));
            const double minX    = funcRef.getFunctionValueCoord(Vec3i(static_cast<int>(pos[0] - m_dxi[0]), static_cast<int>(pos[1]), static_cast<int>(pos[2])));
            const double minY    = funcRef.getFunctionValueCoord(Vec3i(static_cast<int>(pos[0]), static_cast<int>(pos[1] - m_dxi[1]), static_cast<int>(pos[2])));
            const double minZ    = funcRef.getFunctionValueCoord(Vec3i(static_cast<int>(pos[0]), static_cast<int>(pos[1]), static_cast<int>(pos[2] - m_dxi[2])));
            const double maxX    = funcRef.getFunctionValueCoord(Vec3i(static_cast<int>(pos[0] + m_dxi[0]), static_cast<int>(pos[1]), static_cast<int>(pos[2])));
            const double maxY    = funcRef.getFunctionValueCoord(Vec3i(static_cast<int>(pos[0]), static_cast<int>(pos[1] + m_dxi[1]), static_cast<int>(pos[2])));
            const double maxZ    = funcRef.getFunctionValueCoord(Vec3i(static_cast<int>(pos[0]), static_cast<int>(pos[1]), static_cast<int>(pos[2] + m_dxi[2])));

            const double minXY    = funcRef.getFunctionValueCoord(Vec3i(static_cast<int>(pos[0] - m_dxi[0]), static_cast<int>(pos[1] - m_dxi[1]), static_cast<int>(pos[2])));
            const double maxXY    = funcRef.getFunctionValueCoord(Vec3i(static_cast<int>(pos[0] + m_dxi[0]), static_cast<int>(pos[1] + m_dxi[1]), static_cast<int>(pos[2])));
            const double maxXminY = funcRef.getFunctionValueCoord(Vec3i(static_cast<int>(pos[0] + m_dxi[0]), static_cast<int>(pos[1] - m_dxi[1]), static_cast<int>(pos[2])));
            const double minXmaxY = funcRef.getFunctionValueCoord(Vec3i(static_cast<int>(pos[0] - m_dxi[0]), static_cast<int>(pos[1] + m_dxi[1]), static_cast<int>(pos[2])));

            const double minXmaxZ = funcRef.getFunctionValueCoord(Vec3i(static_cast<int>(pos[0] - m_dxi[0]), static_cast<int>(pos[1]), static_cast<int>(pos[2] + m_dxi[2])));
            const double maxXminZ = funcRef.getFunctionValueCoord(Vec3i(static_cast<int>(pos[0] + m_dxi[0]), static_cast<int>(pos[1]), static_cast<int>(pos[2] - m_dxi[2])));
            const double maxXZ    = funcRef.getFunctionValueCoord(Vec3i(static_cast<int>(pos[0] + m_dxi[0]), static_cast<int>(pos[1]), static_cast<int>(pos[2] + m_dxi[2])));
            const double minXZ    = funcRef.getFunctionValueCoord(Vec3i(static_cast<int>(pos[0] - m_dxi[0]), static_cast<int>(pos[1]), static_cast<int>(pos[2] - m_dxi[2])));

            const double minYmaxZ = funcRef.getFunctionValueCoord(Vec3i(static_cast<int>(pos[0]), static_cast<int>(pos[1] - m_dxi[1]), static_cast<int>(pos[2] + m_dxi[2])));
            const double maxYminZ = funcRef.getFunctionValueCoord(Vec3i(static_cast<int>(pos[0]), static_cast<int>(pos[1] + m_dxi[1]), static_cast<int>(pos[2] - m_dxi[2])));
            const double maxYZ    = funcRef.getFunctionValueCoord(Vec3i(static_cast<int>(pos[0]), static_cast<int>(pos[1] + m_dxi[1]), static_cast<int>(pos[2] + m_dxi[2])));
            const double minYZ    = funcRef.getFunctionValueCoord(Vec3i(static_cast<int>(pos[0]), static_cast<int>(pos[1] - m_dxi[1]), static_cast<int>(pos[2] - m_dxi[2])));

            const double dx  = (maxX - minX) * 0.5;
            const double dxx = maxX - 2.0 * central + minX;
            const double dx2 = dx * dx;

            const double dy  = (maxY - minY) * 0.5;
            const double dyy = maxY - 2.0 * central + minY;
            const double dy2 = dy * dy;

            const double dz  = (maxZ - minZ) * 0.5;
            const double dzz = maxZ - 2.0 * central + minZ;
            const double dz2 = dz * dz;

            const double dxy = (maxXY + minXY - maxXminY - minXmaxY) * 0.25;
            const double dxz = (minXmaxZ + maxXminZ - maxXZ - minXZ) * 0.25;
            const double dyz = (minYmaxZ + maxYminZ - maxYZ - minYZ) * 0.25;

            return ((dxx * (dy2 + dz2) + dyy * (dx2 + dz2) + dzz * (dx2 + dy2) -
                     2.0 * dx * dy * dxy - 2.0 * dx * dz * dxz - 2.0 * dy * dz * dyz) /
                    (dx2 + dy2 + dz2 + std::numeric_limits<double>::epsilon()));
        }

        void setDx(const Vec3i& dx, const Vec3d& dxs)
        {
            m_dxi         = dx;
            this->m_dx    = dxs;
            this->m_invDx = Vec3d(1.0 / dxs[0], 1.0 / dxs[1], 1.0 / dxs[2]);
        }

    public:
        const Vec3d& getDx() const { return m_dx; }

        void setFunction(std::shared_ptr<ImplicitGeometry> func) { this->m_func = func; }

    protected:
        std::shared_ptr<ImplicitGeometry> m_func;
        Vec3d m_dx    = Vec3d(1.0, 1.0, 1.0);
        Vec3d m_invDx = Vec3d(1.0, 1.0, 1.0);

    protected:
        Vec3i m_dxi;
};
}