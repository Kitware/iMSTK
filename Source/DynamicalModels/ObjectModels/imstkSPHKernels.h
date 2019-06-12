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
#include <g3log/g3log.hpp>
#include "imstkMath.h"

namespace imstk
{
namespace SPH
{
template<int N>
class CubicKernel {
using VecXr = Eigen::Matrix<Real, N, 1>;

public:
    void setRadius(const Real radius)
    {
        m_radius = radius;
        const auto h2 = m_radius * m_radius;
        const auto h3 = h2 * m_radius;

        // if constexpr (N == 2) {
        if(N == 2)
        {
            m_k = Real(40.0 / 7.0) / (PI * h2);
            m_l = Real(240.0 / 7.0) / (PI * h2);
        }
        else
        {
            m_k = Real(8.0) / (PI * h3);
            m_l = Real(48.0) / (PI * h3);
        }
        m_W_zero = W(VecXr::Zero());
    }

    Real W(const Real r) const
    {
        Real res = 0.;
        const auto q   = r / m_radius;
        if(q <= Real(1.0))
        {
            if(q <= Real(0.5))
            {
                const auto q2 = q * q;
                const auto q3 = q2 * q;
                res = m_k * (Real(6.0) * q3 - Real(6.0) * q2 + Real(1.0));
            }
            else
            {
                res = m_k * (Real(2.0) * std::pow(Real(1.0) - q, 3));
            }
        }
        return res;
    }

    Real W(const VecXr& r) const { return W(r.norm()); }

    VecXr gradW(const VecXr& r) const
    {
        VecXr res = VecXr::Zero();
        const auto r2  = r.squaredNorm();

        if(r2 <= Real(1.0) && r2 > Real(1e-12))
        {
            const auto rl    = Real(sqrt(r2));
            const auto q     = rl / m_radius;
            const auto gradq = r * (Real(1.0) / (rl * m_radius));
            if(q <= Real(0.5))
            {
                res = m_l * q * (Real(3.0) * q - Real(2.0)) * gradq;
            }
            else
            {
                const auto factor = Real(1.0) - q;
                res = m_l * (-factor * factor) * gradq;
            }
        }
        return res;
    }

    Real W_zero() const { return m_W_zero; }

protected:
    Real m_radius;
    Real m_k;
    Real m_l;
    Real m_W_zero;
};


template<int N>
class Poly6Kernel {
using VecXr = Eigen::Matrix<Real, N, 1>;

public:
    void setRadius(const Real radius)
    {
        m_radius  = radius;
        m_radius2 = m_radius * m_radius;

        // if constexpr (N == 2) {
        if(N == 2)
        {
            m_k = Real(4.0) / (PI * std::pow(m_radius, 8));
            m_l = -Real(24.0) / (PI * std::pow(m_radius, 8));
        }
        else
        {
            m_k = Real(315.0) / (Real(64.0) * PI * std::pow(m_radius, 9));
            m_l = -Real(945.0) / (Real(32.0) * PI * std::pow(m_radius, 9));
        }
        m_m      = m_l;
        m_W_zero = W(VecXr::Zero());
    }

    /**
     * W(r,h) = (315/(64 PI h^9))(h^2-|r|^2)^3
     *        = (315/(64 PI h^9))(h^2-r*r)^3
     */
    Real W(const Real r) const
    {
        const auto r2 = r * r;
        return (r2 <= m_radius2) ? std::pow(m_radius2 - r2, 3) * m_k : Real(0);
    }

    Real W(const VecXr& r) const
    {
        const auto r2 = r.squaredNorm();
        return (r2 <= m_radius2) ? std::pow(m_radius2 - r2, 3) * m_k : Real(0);
    }

    /**
     * grad(W(r,h)) = r(-945/(32 PI h^9))(h^2-|r|^2)^2
     *              = r(-945/(32 PI h^9))(h^2-r*r)^2
     */
    VecXr gradW(const VecXr& r) const
    {
        VecXr res = VecXr::Zero();
        const auto r2  = r.squaredNorm();
        if(r2 <= m_radius2 && r2 > Real(1e-12))
        {
            Real tmp = m_radius2 - r2;
            res = m_l * tmp * tmp * r;
        }

        return res;
    }

    /**
     * laplacian(W(r,h)) = (-945/(32 PI h^9))(h^2-|r|^2)(-7|r|^2+3h^2)
     *                   = (-945/(32 PI h^9))(h^2-r*r)(3 h^2-7 r*r)
     */
    Real laplacianW(const VecXr& r) const
    {
        Real res = 0.;
        const auto r2  = r.squaredNorm();
        if(r2 <= m_radius2)
        {
            Real tmp  = m_radius2 - r2;
            Real tmp2 = Real(3.0) * m_radius2 - Real(7.0) * r2;
            res = m_m * tmp * tmp2;
        }

        return res;
    }

    Real W_zero() const { return m_W_zero; }

protected:
    Real m_radius;
    Real m_radius2;
    Real m_k;
    Real m_l;
    Real m_m;
    Real m_W_zero;
};


template<int N>
class SPIkyKernel {
using VecXr = Eigen::Matrix<Real, N, 1>;

public:
    void setRadius(const Real radius)
    {
        m_radius  = radius;
        m_radius2 = m_radius * m_radius;

        // if constexpr (N == 2) {
        if(N == 2)
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
        m_W_zero = W(VecXr::Zero());
    }

    /**
     * W(r,h) = 15/(PI*h^6) * (h-r)^3
     */
    Real W(const Real r) const { return (r <= m_radius) ? std::pow(m_radius - r, 3) * m_k : Real(0); }

    Real W(const VecXr& r) const
    {
        const auto r2 = r.squaredNorm();
        return (r2 <= m_radius2) ? std::pow(m_radius - std::sqrt(r2), 3) * m_k : Real(0);
    }

    /**
     * grad(W(r,h)) = -r(45/(PI*h^6) * (h-r)^2)
     */
    VecXr gradW(const VecXr& r) const
    {
        VecXr res = VecXr::Zero();
        const auto r2  = r.squaredNorm();
        if(r2 <= m_radius2 && r2 > Real(1e-12))
        {
            const auto rl  = std::sqrt(r2);
            const auto hr  = m_radius - rl;
            const auto hr2 = hr * hr;
            res = m_l * hr2 * (r / rl);
        }

        return res;
    }

    Real W_zero() const { return m_W_zero; }

protected:
    Real m_radius;
    Real m_radius2;
    Real m_k;
    Real m_l;
    Real m_W_zero;
};


template<int N>
class CohesionKernel {
using VecXr = Eigen::Matrix<Real, N, 1>;

public:
    void setRadius(const Real radius)
    {
        m_radius  = radius;
        m_radius2 = m_radius * m_radius;

        // if constexpr (N == 2) {
        if(N == 2)
        {
            LOG(FATAL) << "Unimplemented function";
        }
        else
        {
            m_k = Real(32.0) / (PI * std::pow(m_radius, 9));
            m_c = std::pow(m_radius, 6) / Real(64.0);
        }
        m_W_zero = W(VecXr::Zero());
    }

    /**
     * W(r,h) = (32/(PI h^9))(h-r)^3*r^3					if h/2 < r <= h
     *          (32/(PI h^9))(2*(h-r)^3*r^3 - h^6/64		if 0 < r <= h/2
     */
    Real W(const Real r) const
    {
        Real res = 0.;
        const auto r2  = r * r;
        if(r2 <= m_radius2)
        {
            const auto r1 = std::sqrt(r2);
            const auto r3 = r2 * r1;
            if(r1 > Real(0.5) * m_radius)
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

    Real W(const VecXr& r) const
    {
        Real res = 0.;
        const auto r2  = r.squaredNorm();
        if(r2 <= m_radius2)
        {
            const auto r1 = std::sqrt(r2);
            const auto r3 = r2 * r1;
            if(r1 > Real(0.5) * m_radius)
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

    Real W_zero() const { return m_W_zero; }

protected:
    Real m_radius;
    Real m_radius2;
    Real m_k;
    Real m_c;
    Real m_W_zero;
};

template<int N>
class AdhesionKernel {
using VecXr = Eigen::Matrix<Real, N, 1>;

public:
    void setRadius(const Real radius)
    {
        m_radius  = radius;
        m_radius2 = m_radius * m_radius;

        // if constexpr (N == 2) {
        if(N == 2)
        {
            LOG(FATAL) << "Unimplemented function";
        }
        else
        {
            m_k = Real(0.007 / std::pow(m_radius, 3.25));
        }
        m_W_zero = W(VecXr::Zero());
    }

    /**
     * W(r,h) = (0.007/h^3.25)(-4r^2/h + 6r -2h)^0.25					if h/2 < r <= h
     */
    Real W(const Real r) const
    {
        Real res = 0.;
        const auto r2  = r * r;
        if(r2 <= m_radius2)
        {
            const auto r = std::sqrt(r2);
            if(r > Real(0.5) * m_radius)
            {
                res = m_k * std::pow(-4.0 * r2 / m_radius + Real(6.0) * r - Real(2.0) * m_radius, 0.25);
            }
        }
        return res;
    }

    Real W(const VecXr& r) const
    {
        Real res = 0.;
        const auto r2  = r.squaredNorm();
        if(r2 <= m_radius2)
        {
            const auto r = std::sqrt(r2);
            if(r > Real(0.5) * m_radius)
            {
                res = m_k * std::pow(-4.0 * r2 / m_radius + Real(6.0) * r - Real(2.0) * m_radius, 0.25);
            }
        }
        return res;
    }

    Real W_zero() { return m_W_zero; }

protected:
    Real m_radius;
    Real m_radius2;
    Real m_k;
    Real m_W_zero;
};

template<int N>
class ViscosityKernel {
using VecXr = Eigen::Matrix<Real, N, 1>;

public:
    void setRadius(const Real radius)
    {
        m_radius  = radius;
        m_radius2 = radius * radius;
        m_k       = Real(45.0 / PI) / (m_radius2 * m_radius2 * m_radius);
    }

    Real laplace(const VecXr& r) const
    {
        Real res = 0.;
        const auto r2  = r.squaredNorm();
        if(r2 <= m_radius2)
        {
            const auto d = std::sqrt(r2);
            res = m_k * (Real(1) - d / m_radius);
        }
        return res;
    }

protected:
    Real m_radius;
    Real m_radius2;
    Real m_k;
};
} // end namespace SPH

///
/// \brief Struct contains SPH kernels for time integration, using different kernel for different purposes
///

class SPHSimulationKernels
{
public:
    void initialize(const Real kernelRadius)
    {
        m_poly6.setRadius(kernelRadius);
        m_sPIky.setRadius(kernelRadius);
        m_viscosity.setRadius(kernelRadius);
        m_cohesion.setRadius(kernelRadius);
    }

    auto W_zero() const { return m_poly6.W_zero(); }
    auto W(const Vec3r& r) const { return m_poly6.W(r); }
    auto gradW(const Vec3r& r) const { return m_sPIky.gradW(r); }
    auto laplace(const Vec3r& r) const { return m_viscosity.laplace(r); }
    auto cohesionW(const Vec3r& r) const { return m_cohesion.W(r); }

protected:
    SPH::Poly6Kernel<3>     m_poly6;
    SPH::SPIkyKernel<3>     m_sPIky;
    SPH::ViscosityKernel<3> m_viscosity;
    SPH::CohesionKernel<3>  m_cohesion;
};
} // end namespace imstk
