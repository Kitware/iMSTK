/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkPbdFemTetConstraint.h"

namespace imstk
{
bool
PbdFemTetConstraint::initConstraint(
    const Vec3d& p0, const Vec3d& p1, const Vec3d& p2, const Vec3d& p3,
    const PbdParticleId& pIdx0, const PbdParticleId& pIdx1,
    const PbdParticleId& pIdx2, const PbdParticleId& pIdx3,
    std::shared_ptr<PbdFemConstraintConfig> config)
{
    m_particles[0] = pIdx0;
    m_particles[1] = pIdx1;
    m_particles[2] = pIdx2;
    m_particles[3] = pIdx3;

    m_initialElementVolume = (1.0 / 6.0) * (p3 - p0).dot((p1 - p0).cross(p2 - p0));
    m_config     = config;
    m_compliance = 1.0 / (config->m_lambda + 2 * config->m_mu);

    Mat3d m;
    m.col(0) = p0 - p3;
    m.col(1) = p1 - p3;
    m.col(2) = p2 - p3;

    const double det = m.determinant();
    if (fabs(det) > 1.0e-16)
    {
        m_invRestMat = m.inverse();
        return true;
    }

    return false;
}

bool
PbdFemTetConstraint::computeValueAndGradient(PbdState& bodies,
                                             double& c, std::vector<Vec3d>& dcdx)
{
    const Vec3d& p0 = bodies.getPosition(m_particles[0]);
    const Vec3d& p1 = bodies.getPosition(m_particles[1]);
    const Vec3d& p2 = bodies.getPosition(m_particles[2]);
    const Vec3d& p3 = bodies.getPosition(m_particles[3]);

    Mat3d m;
    m.col(0) = p0 - p3;
    m.col(1) = p1 - p3;
    m.col(2) = p2 - p3;

    // deformation gradient (F)
    Mat3d defgrad = m * m_invRestMat;

    // SVD matrices
    Mat3d U    = Mat3d::Identity();
    Mat3d Fhat = Mat3d::Identity();
    Mat3d VT   = Mat3d::Identity();

    Mat3d F = defgrad;

    // If inverted, handle if flag set to true
    if (m_handleInversions && defgrad.determinant() <= 1E-8)
    {
        handleInversions(defgrad, U, Fhat, VT);
        F = Fhat; // diagonalized deformation gradient
    }

    // First Piola-Kirchhoff tensor
    Mat3d P;
    // energy constraint
    double C = 0;

    const auto mu     = m_config->m_mu;
    const auto lambda = m_config->m_lambda;

    switch (m_material)
    {
    // P(F) = F*(2*mu*E + lambda*tr(E)*I)
    // E = (F^T*F - I)/2
    case MaterialType::StVK:
    {
        Mat3d E;
        E(0, 0) = 0.5 * (F(0, 0) * F(0, 0) + F(1, 0) * F(1, 0) + F(2, 0) * F(2, 0) - 1.0);                  // xx
        E(1, 1) = 0.5 * (F(0, 1) * F(0, 1) + F(1, 1) * F(1, 1) + F(2, 1) * F(2, 1) - 1.0);                  // yy
        E(2, 2) = 0.5 * (F(0, 2) * F(0, 2) + F(1, 2) * F(1, 2) + F(2, 2) * F(2, 2) - 1.0);                  // zz
        E(0, 1) = 0.5 * (F(0, 0) * F(0, 1) + F(1, 0) * F(1, 1) + F(2, 0) * F(2, 1));                        // xy
        E(0, 2) = 0.5 * (F(0, 0) * F(0, 2) + F(1, 0) * F(1, 2) + F(2, 0) * F(2, 2));                        // xz
        E(1, 2) = 0.5 * (F(0, 1) * F(0, 2) + F(1, 1) * F(1, 2) + F(2, 1) * F(2, 2));                        // yz
        E(1, 0) = E(0, 1);
        E(2, 0) = E(0, 2);
        E(2, 1) = E(1, 2);

        P = 2 * mu * E;
        double tr = E.trace();
        double lt = lambda * tr;
        P(0, 0) += lt;
        P(1, 1) += lt;
        P(2, 2) += lt;
        P        = F * P;

        C = E(0, 0) * E(0, 0) + E(0, 1) * E(0, 1) + E(0, 2) * E(0, 2)
            + E(1, 0) * E(1, 0) + E(1, 1) * E(1, 1) + E(1, 2) * E(1, 2)
            + E(2, 0) * E(2, 0) + E(2, 1) * E(2, 1) + E(2, 2) * E(2, 2);
        C = mu * C + 0.5 * lambda * tr * tr;

        break;
    }
    // P(F) = (2*mu*(F-R) + lambda*(J-1)*J*F^-T
    case MaterialType::Corotation:
    {
        Eigen::JacobiSVD<Mat3d> svd(F, Eigen::ComputeFullU | Eigen::ComputeFullV);
        Mat3d                   R = svd.matrixU() * svd.matrixV().adjoint();
        Vec3d                   Sigma(svd.singularValues());
        Mat3d                   invFT = svd.matrixU();
        invFT.col(0) /= Sigma(0);
        invFT.col(1) /= Sigma(1);
        invFT.col(2) /= Sigma(2);
        invFT *= svd.matrixV().adjoint();
        double J  = Sigma(0) * Sigma(1) * Sigma(2);
        Mat3d  FR = F - R;

        P = 2 * mu * FR + lambda * (J - 1) * J * invFT;

        C = FR(0, 0) * FR(0, 0) + FR(0, 1) * FR(0, 1) + FR(0, 2) * FR(0, 2)
            + FR(1, 0) * FR(1, 0) + FR(1, 1) * FR(1, 1) + FR(1, 2) * FR(1, 2)
            + FR(2, 0) * FR(2, 0) + FR(2, 1) * FR(2, 1) + FR(2, 2) * FR(2, 2);
        C = mu * C + 0.5 * lambda * (J - 1) * (J - 1);

        break;
    }
    // P(F) = mu*(F - mu*F^-T) + lambda*log(J)F^-T;
    case MaterialType::NeoHookean:
    {
        Mat3d  invFT = F.inverse().transpose();
        double logJ  = log(F.determinant());
        P = mu * (F - invFT) + lambda * logJ * invFT;

        C = F(0, 0) * F(0, 0) + F(0, 1) * F(0, 1) + F(0, 2) * F(0, 2)
            + F(1, 0) * F(1, 0) + F(1, 1) * F(1, 1) + F(1, 2) * F(1, 2)
            + F(2, 0) * F(2, 0) + F(2, 1) * F(2, 1) + F(2, 2) * F(2, 2);

        C = 0.5 * mu * (C - 3) - mu * logJ + 0.5 * lambda * logJ * logJ;

        break;
    }
    case MaterialType::Linear:
    {
        break;
    }
    default:
        break;
    }

    // Rotate P back here. P = U\hat{P}V^{T}
    P = U * P * VT;

    Mat3d gradC = m_initialElementVolume * P * m_invRestMat.transpose();
    c       = C;
    c      *= m_initialElementVolume;
    dcdx[0] = gradC.col(0);
    dcdx[1] = gradC.col(1);
    dcdx[2] = gradC.col(2);
    dcdx[3] = -dcdx[0] - dcdx[1] - dcdx[2];

    return true;
}

void
PbdFemTetConstraint::handleInversions(
    Mat3d& F,
    Mat3d& U,
    Mat3d& Fhat,
    Mat3d& VT) const
{
    // Compute SVD of F and return U and VT. Modify to handle inversions. F = U\hat{F} V^{T}
    Eigen::JacobiSVD<Mat3d> svd(F, Eigen::ComputeFullU | Eigen::ComputeFullV);

    // Save \hat{F}
    Vec3d sigma(svd.singularValues());

    // Store as matrix
    for (int i = 0; i < 3; i++)
    {
        Fhat(i, i) = sigma(i);
    }

    // Save U and V
    U = svd.matrixU();
    Mat3d V = svd.matrixV();

    // Verify that V is a pure rotations
    const double detV = V.determinant();

    // If detV is negative, then V includes a reflection.
    // Switch to reflection by multiplying one column by -1
    if (detV < 0.0)
    {
        double minLambda = IMSTK_DOUBLE_MAX;
        int    column    = 0;
        for (int i = 0; i < 3; i++)
        {
            if (Fhat(i, i) < minLambda)
            {
                column    = i;
                minLambda = Fhat(i, i);
            }
        }

        V.col(column) *= -1.0;
    }

    VT = V.transpose();

    // Check for small singular values
    int count    = 0; // number of small singlar values
    int position = 0; // position of small singular values

    for (int i = 0; i < 3; i++)
    {
        if (fabs(Fhat(i, i)) < 1E-4)
        {
            position = i;
            count++;
        }
    }

    if (count > 0)
    {
        // If more than one singular value is small the element has collapsed
        // to a line or point. To fix set U to identity.
        if (count > 1)
        {
            U.setIdentity();
        }
        else
        {
            U = F * V;

            for (int i = 0; i < 3; i++)
            {
                if (i != position)
                {
                    for (int j = 0; j < 3; j++)
                    {
                        U(j, i) *= 1.0 / Fhat(i, i);
                    }
                }
            }

            // Replace column of U associated with small singular value with
            // new basis orthogonal to other columns of U
            Eigen::Matrix<double, 3, 1, 2> v[2];
            int                            index = 0;
            for (int i = 0; i < 3; i++)
            {
                if (i != position)
                {
                    v[index++] = Eigen::Matrix<double, 3, 1, 2>(U(0, i), U(1, i), U(2, i));
                }
            }

            Vec3d vec = v[0].cross(v[1]).normalized();
            U.col(position) = vec;
        }
    }
    else // No modificaitons required: U = FV\hat{F}^{-1}
    {
        Mat3d FhatInv(Fhat.inverse());
        U = F * V * FhatInv;
    }

    // If detU is negative, then U includes a reflection.
    double detU = U.determinant();

    if (detU < 0.0)
    {
        int    positionu = 0;
        double minLambda = IMSTK_DOUBLE_MAX;
        for (int i = 0; i < 3; i++)
        {
            if (Fhat(i, i) < minLambda)
            {
                positionu = i;
                minLambda = Fhat(i, i);
            }
        }

        // Invert values of smallest singular value and associated column of U
        // This "pushes" the node nearest the uninverted state towards the uninverted state
        Fhat(positionu, positionu) *= -1.0;
        for (int i = 0; i < 3; i++)
        {
            U(i, positionu) *= -1.0;
        }
    }

    // Clamp small singular values of Fhat
    double clamp = 0.577;
    for (int i = 0; i < 3; i++)
    {
        if (Fhat(i, i) < clamp)
        {
            Fhat(i, i) = clamp;
        }
    }
} // end handle tet inversion
}; // namespace imstk