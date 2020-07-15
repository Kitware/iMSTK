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

#include <cmath>
#include <cassert>

#include "imstkMath.h"
#include <g3log/g3log.hpp>

namespace imstk
{
namespace SPH
{
///
/// \class Poly6Kernel
/// \brief The poly6 Kernel
///
template<int N>
class Poly6Kernel
{
using VecXr = Eigen::Matrix<Real, N, 1>;

public:
    Poly6Kernel()
    {
        static_assert(N == 2 || N == 3, "Invalid kernel dimension");
    }

    ///
    /// \brief Set the kernel radius
    ///
    void setRadius(const Real radius)
    {
        m_radius  = radius;
        m_radius2 = m_radius * m_radius;

#pragma warning(push)
#pragma warning(disable:4127)
        if (N == 2)
#pragma warning(pop)
        {
            m_k = Real(4.0) / (PI * std::pow(m_radius, 8));
            m_l = -Real(24.0) / (PI * std::pow(m_radius, 8));
        }
        else
        {
            m_k = Real(315.0) / (Real(64.0) * PI * std::pow(m_radius, 9));
            m_l = -Real(945.0) / (Real(32.0) * PI * std::pow(m_radius, 9));
        }
        m_m  = m_l;
        m_W0 = W(VecXr::Zero());
    }

    ///
    /// \brief Compute weight value
    /// W(r,h) = (315/(64 PI h^9))(h^2-|r|^2)^3
    ///
    Real W(const Real r) const
    {
        const auto r2 = r * r;
        return (r2 <= m_radius2) ? std::pow(m_radius2 - r2, 3) * m_k : Real(0);
    }

    ///
    /// \brief Compute weight value
    /// W(r,h) = (315/(64 PI h^9))(h^2-|r|^2)^3
    ///
    Real W(const VecXr& r) const
    {
        const auto r2 = r.squaredNorm();
        return (r2 <= m_radius2) ? std::pow(m_radius2 - r2, 3) * m_k : Real(0);
    }

    ///
    /// \brief Get W(0)
    ///
    Real W0() const { return m_W0; }

    ///
    /// \brief Compute weight gradient
    /// grad(W(r,h)) = r(-945/(32 PI h^9))(h^2-|r|^2)^2
    ///
    VecXr gradW(const VecXr& r) const
    {
        VecXr      res = VecXr::Zero();
        const auto r2  = r.squaredNorm();
        if (r2 <= m_radius2 && r2 > Real(1e-12))
        {
            Real tmp = m_radius2 - r2;
            res = m_l * tmp * tmp * r;
        }

        return res;
    }

    ///
    /// \brief Compute laplacian
    /// laplacian(W(r,h)) = (-945/(32 PI h^9))(h^2-|r|^2)(-7|r|^2+3h^2)
    ///
    Real laplacian(const VecXr& r) const
    {
        Real       res = 0.;
        const auto r2  = r.squaredNorm();
        if (r2 <= m_radius2)
        {
            Real tmp  = m_radius2 - r2;
            Real tmp2 = Real(3.0) * m_radius2 - Real(7.0) * r2;
            res = m_m * tmp * tmp2;
        }

        return res;
    }

protected:
    Real m_radius;  ///> Kernel radius
    Real m_radius2; ///> Kernel radius squared
    Real m_k;       ///> Kernel coefficient for W()
    Real m_l;       ///> Kernel coefficient for gradW()
    Real m_m;       ///> Kernel coefficient for laplacian()
    Real m_W0;      ///> Precomputed W(0)
};

///
/// \class SpikyKernel
/// \brief Spiky Kernel
///
template<int N>
class SpikyKernel
{
using VecXr = Eigen::Matrix<Real, N, 1>;

public:
    SpikyKernel()
    {
        static_assert(N == 2 || N == 3, "Invalid kernel dimension");
    }

    ///
    /// \brief Set the kernel radius
    ///
    void setRadius(const Real radius)
    {
        m_radius  = radius;
        m_radius2 = m_radius * m_radius;

#pragma warning(push)
#pragma warning(disable:4127)
        if (N == 2)
#pragma warning(pop)
        {
            const auto radius5 = std::pow(m_radius, 5);
            m_k = Real(10.0) / (PI * radius5);
            m_l = -Real(30.0) / (PI * radius5);
        }
        else
        {
            const auto radius6 = std::pow(m_radius, 6);
            m_k = Real(15.0) / (PI * radius6);
            m_l = -Real(45.0) / (PI * radius6);
        }
        m_W0 = W(VecXr::Zero());
    }

    ///
    /// \brief Compute weight value
    /// W(r,h) = 15/(PI*h^6) * (h-r)^3
    ///
    Real W(const Real r) const { return (r <= m_radius) ? std::pow(m_radius - r, 3) * m_k : Real(0); }

    ///
    /// \brief Compute weight value
    /// W(r,h) = 15/(PI*h^6) * (h-r)^3
    ///
    Real W(const VecXr& r) const
    {
        const auto r2 = r.squaredNorm();
        return (r2 <= m_radius2) ? std::pow(m_radius - std::sqrt(r2), 3) * m_k : Real(0);
    }

    ///
    /// \brief Get W(0)
    ///
    Real W0() const { return m_W0; }

    ///
    /// \brief Compute weight gradient
    /// grad(W(r,h)) = -r(45/(PI*h^6) * (h-r)^2)
    ///
    VecXr gradW(const VecXr& r) const
    {
        VecXr      res = VecXr::Zero();
        const auto r2  = r.squaredNorm();
        if (r2 <= m_radius2 && r2 > Real(1e-12))
        {
            const auto rl  = std::sqrt(r2);
            const auto hr  = m_radius - rl;
            const auto hr2 = hr * hr;
            res = m_l * hr2 * (r / rl);
        }

        return res;
    }

protected:
    Real m_radius;  ///> Kernel radius
    Real m_radius2; ///> Kernel radius squared
    Real m_k;       ///> Kernel coefficient for W()
    Real m_l;       ///> Kernel coefficient for gradW()
    Real m_W0;      ///> Precomputed W(0)
};

///
/// \class CohesionKernel
/// \brief Cohesion Kernel
///
template<int N>
class CohesionKernel
{
using VecXr = Eigen::Matrix<Real, N, 1>;

public:
    CohesionKernel()
    {
        static_assert(N == 3, "Invalid kernel dimension");
    }

    ///
    /// \brief Set the kernel radius
    ///
    void setRadius(const Real radius)
    {
        m_radius  = radius;
        m_radius2 = m_radius * m_radius;

#pragma warning(push)
#pragma warning(disable:4127)
        CHECK(N != 2) << "Unimplemented function";
#pragma warning(pop)

        m_k  = Real(32.0) / (PI * std::pow(m_radius, 9));
        m_c  = std::pow(m_radius, 6) / Real(64.0);
        m_W0 = W(VecXr::Zero());
    }

    ///
    /// \brief Compute weight value
    /// W(r,h) = (32/(PI h^9))(h-r)^3*r^3					if h/2 < r <= h,
    ///          (32/(PI h^9))(2*(h-r)^3*r^3 - h^6/64		if 0 < r <= h/2
    Real W(const Real r) const
    {
        Real       res = 0.;
        const auto r2  = r * r;
        if (r2 <= m_radius2)
        {
            const auto r1 = std::sqrt(r2);
            const auto r3 = r2 * r1;
            if (r1 > Real(0.5) * m_radius)
            {
                res = m_k * std::pow(m_radius - r1, 3) * r3;
            }
            else
            {
                res = m_k * Real(2.0) * std::pow(m_radius - r1, 3) * r3 - m_c;
            }
        }
        return res;
    }

    ///
    /// \brief Compute weight value
    /// W(r,h) = (32/(PI h^9))(h-r)^3*r^3					if h/2 < r <= h,
    ///          (32/(PI h^9))(2*(h-r)^3*r^3 - h^6/64		if 0 < r <= h/2
    Real W(const VecXr& r) const
    {
        Real       res = 0.;
        const auto r2  = r.squaredNorm();
        if (r2 <= m_radius2)
        {
            const auto r1 = std::sqrt(r2);
            const auto r3 = r2 * r1;
            if (r1 > Real(0.5) * m_radius)
            {
                res = m_k * std::pow(m_radius - r1, 3) * r3;
            }
            else
            {
                res = m_k * Real(2.0) * std::pow(m_radius - r1, 3) * r3 - m_c;
            }
        }
        return res;
    }

    ///
    /// \brief Get W(0)
    ///
    Real W0() const { return m_W0; }

protected:
    Real m_radius;  ///> Kernel radius
    Real m_radius2; ///> Kernel radius squared
    Real m_k;       ///> Kernel coefficient for W()
    Real m_c;       ///> Kernel coefficient for W()
    Real m_W0;      ///> Precomputed W(0)
};

///
/// \class AdhesionKernel
/// \brief Adhesion kernel
///
template<int N>
class AdhesionKernel
{
using VecXr = Eigen::Matrix<Real, N, 1>;

public:
    AdhesionKernel()
    {
        static_assert(N == 3, "Invalid kernel dimension");
    }

    ///
    /// \brief Set the kernel radius
    ///
    void setRadius(const Real radius)
    {
        m_radius  = radius;
        m_radius2 = m_radius * m_radius;

        CHECK(N != 2) << "Unimplemented function";

        m_k  = Real(0.007 / std::pow(m_radius, 3.25));
        m_W0 = W(VecXr::Zero());
    }

    ///
    /// \brief Compute weight value
    ///  W(r,h) = (0.007/h^3.25)(-4r^2/h + 6r -2h)^0.25					if h/2 < r <= h
    ///
    Real W(const Real r) const
    {
        Real       res = 0.;
        const auto r2  = r * r;
        if (r2 <= m_radius2)
        {
            const auto r = std::sqrt(r2);
            if (r > Real(0.5) * m_radius)
            {
                res = m_k * std::pow(-4.0 * r2 / m_radius + Real(6.0) * r - Real(2.0) * m_radius, 0.25);
            }
        }
        return res;
    }

    ///
    /// \brief Compute weight value
    ///  W(r,h) = (0.007/h^3.25)(-4r^2/h + 6r -2h)^0.25					if h/2 < r <= h
    ///
    Real W(const VecXr& r) const
    {
        Real       res = 0.;
        const auto r2  = r.squaredNorm();
        if (r2 <= m_radius2)
        {
            const auto r = std::sqrt(r2);
            if (r > Real(0.5) * m_radius)
            {
                res = m_k * std::pow(-4.0 * r2 / m_radius + Real(6.0) * r - Real(2.0) * m_radius, 0.25);
            }
        }
        return res;
    }

    ///
    /// \brief Get W(0)
    ///
    Real W0() const { return m_W0; }

protected:
    Real m_radius;  ///> Kernel radius
    Real m_radius2; ///> Kernel radius squared
    Real m_k;       ///> Kernel coefficient for W()
    Real m_W0;      ///> Precomputed W(0)
};

///
/// \class ViscosityKernel
/// \brief Viscosity kernel
///
template<int N>
class ViscosityKernel
{
using VecXr = Eigen::Matrix<Real, N, 1>;

public:
    ViscosityKernel()
    {
        static_assert(N == 2 || N == 3, "Invalid kernel dimension");
    }

    ///
    /// \brief Set the kernel radius
    ///
    void setRadius(const Real radius)
    {
        m_radius  = radius;
        m_radius2 = radius * radius;
        m_k       = Real(45.0 / PI) / (m_radius2 * m_radius2 * m_radius2);
    }

    ///
    /// \brief Compute laplacian
    /// Laplace(r) = (45/PI/h^6) * (h - |r|)
    ///
    Real laplace(const VecXr& r) const
    {
        Real       res = 0.;
        const auto r2  = r.squaredNorm();
        if (r2 <= m_radius2)
        {
            const auto d = std::sqrt(r2);
            res = m_k * (m_radius - d);
        }
        return res;
    }

protected:
    Real m_radius;  ///> Kernel radius
    Real m_radius2; ///> Kernel radius squared
    Real m_k;       ///> Kernel coefficient for laplacian()
};
} // end namespace SPH

///
/// \brief Class contains SPH kernels for time integration,
/// using different kernel for different purposes
///
class SPHSimulationKernels
{
public:
    ///
    /// \brief Initialize with kernel radius \p kernelRadius
    ///
    void initialize(const Real kernelRadius)
    {
        m_poly6.setRadius(kernelRadius);
        m_spiky.setRadius(kernelRadius);
        m_viscosity.setRadius(kernelRadius);
        m_cohesion.setRadius(kernelRadius);
    }

    ///
    /// \brief Compute weight W(0) using poly6 kernel
    ///
    Real W0() const { return m_poly6.W0(); }

    ///
    /// \brief Compute weight W using poly6 kernel
    ///
    Real W(const Vec3r& r) const { return m_poly6.W(r); }

    ///
    /// \brief Compute gradW using spiky kernel
    ///
    Vec3r gradW(const Vec3r& r) const { return m_spiky.gradW(r); }

    ///
    /// \brief Compute laplacian using viscosity kernel
    ///
    Real laplace(const Vec3r& r) const { return m_viscosity.laplace(r); }

    ///
    /// \brief Compute cohesion W using cohesion kernel
    ///
    Real cohesionW(const Vec3r& r) const { return m_cohesion.W(r); }

protected:
    SPH::Poly6Kernel<3>     m_poly6;
    SPH::SpikyKernel<3>     m_spiky;
    SPH::ViscosityKernel<3> m_viscosity;
    SPH::CohesionKernel<3>  m_cohesion;
};
} // end namespace imstk
