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

#include "imstkMath.h"
#include "imstkLogger.h"

namespace imstk
{
namespace sph
{
///
/// \class Poly6Kernel
/// \brief The poly6 Kernel
///
template<int N>
class Poly6Kernel
{
using VecXd = Eigen::Matrix<double, N, 1>;

public:
    Poly6Kernel()
    {
        static_assert(N == 2 || N == 3, "Invalid kernel dimension");
    }

    ///
    /// \brief Set the kernel radius
    ///
    void setRadius(const double radius)
    {
        m_radius = radius;
        m_radiusSquared = m_radius * m_radius;

#ifdef WIN32
#pragma warning(push)
#pragma warning(disable:4127)
#endif
        if (N == 2)
#ifdef WIN32
#pragma warning(pop)
#endif
        {
            m_k = 4.0 / (PI * std::pow(m_radius, 8));
            m_l = -24.0 / (PI * std::pow(m_radius, 8));
        }
        else
        {
            m_k = 315.0 / (64.0 * PI * std::pow(m_radius, 9));
            m_l = -945.0 / (32.0 * PI * std::pow(m_radius, 9));
        }
        m_m  = m_l;
        m_W0 = W(VecXd::Zero());
    }

    ///
    /// \brief Compute weight value
    /// W(r,h) = (315/(64 PI h^9))(h^2-|r|^2)^3
    ///
    double W(const double r) const
    {
        const double r2 = r * r;
        const double rd = m_radiusSquared - r2;
        return (r2 <= m_radiusSquared) ? rd * rd * rd * m_k : 0.0;
    }

    ///
    /// \brief Compute weight value
    /// W(r,h) = (315/(64 PI h^9))(h^2-|r|^2)^3
    ///
    double W(const VecXd& r) const
    {
        const double r2 = r.squaredNorm();
        const double rd = m_radiusSquared - r2;
        return (r2 <= m_radiusSquared) ? rd * rd * rd * m_k : 0.0;
    }

    ///
    /// \brief Get W(0)
    ///
    double W0() const { return m_W0; }

    ///
    /// \brief Compute weight gradient
    /// grad(W(r,h)) = r(-945/(32 PI h^9))(h^2-|r|^2)^2
    ///
    VecXd gradW(const VecXd& r) const
    {
        VecXd        res = VecXd::Zero();
        const double r2  = r.squaredNorm();
        if (r2 <= m_radiusSquared && r2 > 1.0e-12)
        {
            double tmp = m_radiusSquared - r2;
            res = m_l * tmp * tmp * r;
        }

        return res;
    }

    ///
    /// \brief Compute laplacian
    /// laplacian(W(r,h)) = (-945/(32 PI h^9))(h^2-|r|^2)(-7|r|^2+3h^2)
    ///
    double laplacian(const VecXd& r) const
    {
        double       res = 0.;
        const double r2  = r.squaredNorm();
        if (r2 <= m_radiusSquared)
        {
            double tmp  = m_radiusSquared - r2;
            double tmp2 = 3.0 * m_radiusSquared - 7.0 * r2;
            res = m_m * tmp * tmp2;
        }

        return res;
    }

protected:
    double m_radius;        ///> Kernel radius
    double m_radiusSquared; ///> Kernel radius squared
    double m_k;             ///> Kernel coefficient for W()
    double m_l;             ///> Kernel coefficient for gradW()
    double m_m;             ///> Kernel coefficient for laplacian()
    double m_W0;            ///> Precomputed W(0)
};

///
/// \class SpikyKernel
/// \brief Spiky Kernel
///
template<int N>
class SpikyKernel
{
using VecXd = Eigen::Matrix<double, N, 1>;

public:
    SpikyKernel()
    {
        static_assert(N == 2 || N == 3, "Invalid kernel dimension");
    }

    ///
    /// \brief Set the kernel radius
    ///
    void setRadius(const double radius)
    {
        m_radius = radius;
        m_radiusSquared = m_radius * m_radius;

#ifdef WIN32
#pragma warning(push)
#pragma warning(disable:4127)
#endif
        if (N == 2)
#ifdef WIN32
#pragma warning(pop)
#endif
        {
            const double radius5 = std::pow(m_radius, 5);
            m_k = 10.0 / (PI * radius5);
            m_l = -30.0 / (PI * radius5);
        }
        else
        {
            const double radius6 = std::pow(m_radius, 6);
            m_k = 15.0 / (PI * radius6);
            m_l = -45.0 / (PI * radius6);
        }
        m_W0 = W(VecXd::Zero());
    }

    ///
    /// \brief Compute weight value
    /// W(r,h) = 15/(PI*h^6) * (h-r)^3
    ///
    double W(const double r) const
    {
        const double rd = m_radius - r;
        return (r <= m_radius) ? rd * rd * rd * m_k : 0.0;
    }

    ///
    /// \brief Compute weight value
    /// W(r,h) = 15/(PI*h^6) * (h-r)^3
    ///
    double W(const VecXd& r) const
    {
        const double r2 = r.squaredNorm();
        const double rd = m_radius - std::sqrt(r2);
        return (r2 <= m_radiusSquared) ? rd * rd * rd * m_k : 0.0;
    }

    ///
    /// \brief Get W(0)
    ///
    double W0() const { return m_W0; }

    ///
    /// \brief Compute weight gradient
    /// grad(W(r,h)) = -r(45/(PI*h^6) * (h-r)^2)
    ///
    VecXd gradW(const VecXd& r) const
    {
        VecXd      res = VecXd::Zero();
        const auto r2  = r.squaredNorm();
        if (r2 <= m_radiusSquared && r2 > 1.0e-12)
        {
            const double rl  = std::sqrt(r2);
            const double hr  = m_radius - rl;
            const double hr2 = hr * hr;
            res = m_l * hr2 * (r / rl);
        }

        return res;
    }

protected:
    double m_radius;        ///> Kernel radius
    double m_radiusSquared; ///> Kernel radius squared
    double m_k;             ///> Kernel coefficient for W()
    double m_l;             ///> Kernel coefficient for gradW()
    double m_W0;            ///> Precomputed W(0)
};

///
/// \class CohesionKernel
/// \brief Cohesion Kernel
///
template<int N>
class CohesionKernel
{
using VecXd = Eigen::Matrix<double, N, 1>;

public:
    CohesionKernel()
    {
        static_assert(N == 3, "Invalid kernel dimension");
    }

    ///
    /// \brief Set the kernel radius
    ///
    void setRadius(const double radius)
    {
        m_radius = radius;
        m_radiusSquared = m_radius * m_radius;

#ifdef WIN32
#pragma warning(push)
#pragma warning(disable:4127)
#endif
        CHECK(N != 2) << "Unimplemented function";
#ifdef WIN32
#pragma warning(pop)
#endif

        m_k  = 32.0 / (PI * std::pow(m_radius, 9));
        m_c  = std::pow(m_radius, 6) / 64.0;
        m_W0 = W(VecXd::Zero());
    }

    ///
    /// \brief Compute weight value
    /// W(r,h) = (32/(PI h^9))(h-r)^3*r^3					if h/2 < r <= h,
    ///          (32/(PI h^9))(2*(h-r)^3*r^3 - h^6/64		if 0 < r <= h/2
    double W(const double r) const
    {
        double       res = 0.;
        const double r2  = r * r;
        if (r2 <= m_radiusSquared)
        {
            const double r1 = std::sqrt(r2);
            const double r3 = r2 * r1;
            if (r1 > 0.5 * m_radius)
            {
                const double rd = m_radius - r1;
                res = m_k * rd * rd * rd * r3;
            }
            else
            {
                const double rd = m_radius - r1;
                res = m_k * 2.0 * rd * rd * rd * r3 - m_c;
            }
        }
        return res;
    }

    ///
    /// \brief Compute weight value
    /// W(r,h) = (32/(PI h^9))(h-r)^3*r^3					if h/2 < r <= h,
    ///          (32/(PI h^9))(2*(h-r)^3*r^3 - h^6/64		if 0 < r <= h/2
    double W(const VecXd& r) const
    {
        double       res = 0.;
        const double r2  = r.squaredNorm();
        if (r2 <= m_radiusSquared)
        {
            const double r1 = std::sqrt(r2);
            const double r3 = r2 * r1;
            if (r1 > 0.5 * m_radius)
            {
                const double rd = m_radius - r1;
                res = m_k * rd * rd * rd * r3;
            }
            else
            {
                const double rd = m_radius - r1;
                res = m_k * 2.0 * rd * rd * rd * r3 - m_c;
            }
        }
        return res;
    }

    ///
    /// \brief Get W(0)
    ///
    double W0() const { return m_W0; }

protected:
    double m_radius;        ///> Kernel radius
    double m_radiusSquared; ///> Kernel radius squared
    double m_k;             ///> Kernel coefficient for W()
    double m_c;             ///> Kernel coefficient for W()
    double m_W0;            ///> Precomputed W(0)
};

///
/// \class AdhesionKernel
/// \brief Adhesion kernel
///
template<int N>
class AdhesionKernel
{
using VecXd = Eigen::Matrix<double, N, 1>;

public:
    AdhesionKernel()
    {
        static_assert(N == 3, "Invalid kernel dimension");
    }

    ///
    /// \brief Set the kernel radius
    ///
    void setRadius(const double radius)
    {
        m_radius = radius;
        m_radiusSquared = m_radius * m_radius;

        CHECK(N != 2) << "Unimplemented function";

        m_k  = 0.007 / std::pow(m_radius, 3.25);
        m_W0 = W(VecXd::Zero());
    }

    ///
    /// \brief Compute weight value
    ///  W(r,h) = (0.007/h^3.25)(-4r^2/h + 6r -2h)^0.25					if h/2 < r <= h
    ///
    double W(const double r) const
    {
        double       res = 0.;
        const double r2  = r * r;
        if (r2 <= m_radiusSquared)
        {
            const double r = std::sqrt(r2);
            if (r > 0.5 * m_radius)
            {
                res = m_k * std::pow(-4.0 * r2 / m_radius + 6.0 * r - 2.0 * m_radius, 0.25);
            }
        }
        return res;
    }

    ///
    /// \brief Compute weight value
    ///  W(r,h) = (0.007/h^3.25)(-4r^2/h + 6r -2h)^0.25					if h/2 < r <= h
    ///
    double W(const VecXd& r) const
    {
        double       res = 0.;
        const double r2  = r.squaredNorm();
        if (r2 <= m_radiusSquared)
        {
            const double r = std::sqrt(r2);
            if (r > 0.5 * m_radius)
            {
                res = m_k * std::pow(-4.0 * r2 / m_radius + 6.0 * r - 2.0 * m_radius, 0.25);
            }
        }
        return res;
    }

    ///
    /// \brief Get W(0)
    ///
    double W0() const { return m_W0; }

protected:
    double m_radius;        ///> Kernel radius
    double m_radiusSquared; ///> Kernel radius squared
    double m_k;             ///> Kernel coefficient for W()
    double m_W0;            ///> Precomputed W(0)
};

///
/// \class ViscosityKernel
/// \brief Viscosity kernel
///
template<int N>
class ViscosityKernel
{
using VecXd = Eigen::Matrix<double, N, 1>;

public:
    ViscosityKernel()
    {
        static_assert(N == 2 || N == 3, "Invalid kernel dimension");
    }

    ///
    /// \brief Set the kernel radius
    ///
    void setRadius(const double radius)
    {
        m_radius = radius;
        m_radiusSquared = radius * radius;
        m_k = (45.0 / PI) / (m_radiusSquared * m_radiusSquared * m_radiusSquared);
    }

    ///
    /// \brief Compute laplacian
    /// Laplace(r) = (45/PI/h^6) * (h - |r|)
    ///
    double laplace(const VecXd& r) const
    {
        double       res = 0.;
        const double r2  = r.squaredNorm();
        if (r2 <= m_radiusSquared)
        {
            const double d = std::sqrt(r2);
            res = m_k * (m_radius - d);
        }
        return res;
    }

protected:
    double m_radius;        ///> Kernel radius
    double m_radiusSquared; ///> Kernel radius squared
    double m_k;             ///> Kernel coefficient for laplacian()
};
} // end namespace SPH

///
/// \brief Class contains SPH kernels for time integration,
/// using different kernel for different purposes
///
class SphSimulationKernels
{
public:
    ///
    /// \brief Initialize with kernel radius \p kernelRadius
    ///
    void initialize(const double kernelRadius)
    {
        m_poly6.setRadius(kernelRadius);
        m_spiky.setRadius(kernelRadius);
        m_viscosity.setRadius(kernelRadius);
        m_cohesion.setRadius(kernelRadius);
    }

    ///
    /// \brief Compute weight W(0) using poly6 kernel
    ///
    double W0() const { return m_poly6.W0(); }

    ///
    /// \brief Compute weight W using poly6 kernel
    ///
    double W(const Vec3d& r) const { return m_poly6.W(r); }

    ///
    /// \brief Compute gradW using spiky kernel
    ///
    Vec3d gradW(const Vec3d& r) const { return m_spiky.gradW(r); }

    ///
    /// \brief Compute laplacian using viscosity kernel
    ///
    double laplace(const Vec3d& r) const { return m_viscosity.laplace(r); }

    ///
    /// \brief Compute cohesion W using cohesion kernel
    ///
    double cohesionW(const Vec3d& r) const { return m_cohesion.W(r); }

protected:
    sph::Poly6Kernel<3>     m_poly6;
    sph::SpikyKernel<3>     m_spiky;
    sph::ViscosityKernel<3> m_viscosity;
    sph::CohesionKernel<3>  m_cohesion;
};
} // end namespace imstk
