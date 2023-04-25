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
    PbdFemConstraintConfig config)
{
    m_particles[0] = pIdx0;
    m_particles[1] = pIdx1;
    m_particles[2] = pIdx2;
    m_particles[3] = pIdx3;

    m_initialElementVolume = tetVolume(p0, p1, p2, p3);
    m_config     = config;
    m_compliance = 1.0 / (config.m_lambda + 2 * config.m_mu);

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

    const double mu     = m_config.m_mu;
    const double lambda = m_config.m_lambda;

    switch (m_material)
    {
    // P(F) = F*(2*mu*E + lambda*tr(E)*I)
    // E = (F^T*F - I)/2
    case MaterialType::StVK:
    {
        Mat3d I = Mat3d::Identity();
        Mat3d E = 0.5 * (F.transpose() * F - I);

        P = F * (2.0 * mu * E + lambda * E.trace() * I);

        // C here is strain energy (Often denoted as W in literature)
        // for the StVK mondel W = mu[tr(E^{T}E)] + 0.5*lambda*(tr(E))^2
        C = mu * ((E.transpose() * E).trace()) + 0.5 * lambda * (E.trace() * E.trace());

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
    // P(F) = mu*(F - mu*F^-T) + 0.5*lambda*log^{2}(J)F^-T;
    // C = 0.5*mu*(I1 - log(I3) - 3) + (lambda/8)*log^{2}(I3)
    case MaterialType::NeoHookean:
    {
        // First invariant
        double I1 = (F * F.transpose()).trace();

        // Third invariant
        double I3    = (F.transpose() * F).determinant();
        auto   logI3 = log(I3);

        auto F_invT = F.inverse().transpose();

        P = mu * (F - F_invT) + 0.5 * lambda * logI3 * F_invT;

        C = 0.5 * mu * (I1 - logI3 - 3.0) + 0.125 * lambda * (logI3 * logI3);

        break;
    }
    // e = 0.5*(F*F^{T} - I)
    // P = 2*mu*e + lambda*tr(e)*I
    case MaterialType::Linear:
    {
        Mat3d I = Mat3d::Identity();

        Mat3d e = 0.5 * (F * F.transpose() - I);

        P = 2.0 * mu * e + lambda * e.trace() * I;
        C = mu * (e * e).trace() + 0.5 * lambda * e.trace() * e.trace();

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
        U = F * V * Fhat.inverse();
    }

    // If detU is negative, then U includes a reflection.
    const double detU = U.determinant();

    if (detU < 0.0)
    {
        int    positionU = 0;
        double minLambda = IMSTK_DOUBLE_MAX;
        for (int i = 0; i < 3; i++)
        {
            if (Fhat(i, i) < minLambda)
            {
                positionU = i;
                minLambda = Fhat(i, i);
            }
        }

        // Invert values of smallest singular value and associated column of U
        // This "pushes" the node nearest the uninverted state towards the uninverted state
        Fhat(positionU, positionU) *= -1.0;
        for (int i = 0; i < 3; i++)
        {
            U(i, positionU) *= -1.0;
        }
    }

    // Clamp small singular values of Fhat
    const double clamp = 0.577;
    for (int i = 0; i < 3; i++)
    {
        if (Fhat(i, i) < clamp)
        {
            Fhat(i, i) = clamp;
        }
    }
} // end handle tet inversion
}; // namespace imstk