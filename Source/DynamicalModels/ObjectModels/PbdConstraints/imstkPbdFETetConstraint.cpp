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

#include "imstkPbdFETetConstraint.h"
#include "imstkPbdModel.h"
#include "imstkLogUtility.h"

namespace  imstk
{
bool
PbdFEMTetConstraint::initConstraint(PbdModel& model,
                                    const size_t& pIdx1, const size_t& pIdx2,
                                    const size_t& pIdx3, const size_t& pIdx4)
{
    m_vertexIds[0] = pIdx1;
    m_vertexIds[1] = pIdx2;
    m_vertexIds[2] = pIdx3;
    m_vertexIds[3] = pIdx4;

    auto state = model.getInitialState();

    const Vec3d& p0 = state->getVertexPosition(pIdx1);
    const Vec3d& p1 = state->getVertexPosition(pIdx2);
    const Vec3d& p2 = state->getVertexPosition(pIdx3);
    const Vec3d& p3 = state->getVertexPosition(pIdx4);

    m_elementVolume = (1.0 / 6.0) * (p3 - p0).dot((p1 - p0).cross(p2 - p0));

    Mat3d m;
    m.col(0) = p0 - p3;
    m.col(1) = p1 - p3;
    m.col(2) = p2 - p3;

    const double det = m.determinant();
    if (fabs(det) > m_epsilon)
    {
        m_invRestMat = m.inverse();
        return true;
    }

    return false;
}

bool
PbdFEMTetConstraint::solvePositionConstraint(PbdModel& model)
{
    const auto i1 = m_vertexIds[0];
    const auto i2 = m_vertexIds[1];
    const auto i3 = m_vertexIds[2];
    const auto i4 = m_vertexIds[3];

    auto state = model.getCurrentState();

    Vec3d& p0 = state->getVertexPosition(i1);
    Vec3d& p1 = state->getVertexPosition(i2);
    Vec3d& p2 = state->getVertexPosition(i3);
    Vec3d& p3 = state->getVertexPosition(i4);

    //double currentVolume = (1.0 / 6.0) * (p3 - p0).dot((p1 - p0).cross(p2 - p0));

    Mat3d m;
    m.col(0) = p0 - p3;
    m.col(1) = p1 - p3;
    m.col(2) = p2 - p3;

    // deformation gradient
    const Mat3d F = m * m_invRestMat;
    // First Piola-Kirchhoff tensor
    Mat3d P;
    // energy constraint
    double C = 0;

    const auto mu     = model.getParameters()->m_mu;
    const auto lambda = model.getParameters()->m_lambda;

    switch (m_material)
    {
    // P(F) = F*(2*mu*E + lambda*tr(E)*I)
    // E = (F^T*F - I)/2
    case MaterialType::StVK:
    {
        Mat3d E;
        E(0, 0) = 0.5 * (F(0, 0) * F(0, 0) + F(1, 0) * F(1, 0) + F(2, 0) * F(2, 0) - 1.0);    // xx
        E(1, 1) = 0.5 * (F(0, 1) * F(0, 1) + F(1, 1) * F(1, 1) + F(2, 1) * F(2, 1) - 1.0);    // yy
        E(2, 2) = 0.5 * (F(0, 2) * F(0, 2) + F(1, 2) * F(1, 2) + F(2, 2) * F(2, 2) - 1.0);    // zz
        E(0, 1) = 0.5 * (F(0, 0) * F(0, 1) + F(1, 0) * F(1, 1) + F(2, 0) * F(2, 1));          // xy
        E(0, 2) = 0.5 * (F(0, 0) * F(0, 2) + F(1, 0) * F(1, 2) + F(2, 0) * F(2, 2));          // xz
        E(1, 2) = 0.5 * (F(0, 1) * F(0, 2) + F(1, 1) * F(1, 2) + F(2, 1) * F(2, 2));          // yz
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
    {
        LOG(FATAL) << "Material type not supported ! \n";
        break;
    }
    }

    const double im1 = model.getInvMass(i1);
    const double im2 = model.getInvMass(i2);
    const double im3 = model.getInvMass(i3);
    const double im4 = model.getInvMass(i4);

    Mat3d gradC = m_elementVolume * P * m_invRestMat.transpose();

    double sum = im1 * gradC.col(0).squaredNorm()
                 + im2 * gradC.col(1).squaredNorm()
                 + im3 * gradC.col(2).squaredNorm()
                 + im4 * (gradC.col(0) + gradC.col(1) + gradC.col(2)).squaredNorm();

    if (sum < m_epsilon)
    {
        return false;
    }

    C = C * m_elementVolume;

    const double s = C / sum;

    if (im1 > 0)
    {
        p0 += -s* im1* gradC.col(0);
    }

    if (im2 > 0)
    {
        p1 += -s* im2* gradC.col(1);
    }

    if (im3 > 0)
    {
        p2 += -s* im3* gradC.col(2);
    }

    if (im4 > 0)
    {
        p3 += s * im4 * (gradC.col(0) + gradC.col(1) + gradC.col(2));
    }

    return true;
}
} // imstk
