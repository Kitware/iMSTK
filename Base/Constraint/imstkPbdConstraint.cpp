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

#include "imstkPbdConstraint.h"
#include "imstkPbdModel.h"

namespace  imstk
{

using Vec3d = Eigen::Vector3d;
using Mat3d = Eigen::Matrix3d;

bool
FEMTetConstraint::initConstraint (PositionBasedModel &model,
                                  const unsigned int &pIdx1, const unsigned int &pIdx2,
                                  const unsigned int &pIdx3, const unsigned int &pIdx4)
{
    m_bodies[0] = pIdx1;
    m_bodies[1] = pIdx2;
    m_bodies[2] = pIdx3;
    m_bodies[3] = pIdx4;

    auto state = model.getState();

    Vec3d &p0 = state->getInitialVertexPosition(pIdx1);
    Vec3d &p1 = state->getInitialVertexPosition(pIdx2);
    Vec3d &p2 = state->getInitialVertexPosition(pIdx3);
    Vec3d &p3 = state->getInitialVertexPosition(pIdx4);

    m_Volume = (1.0 / 6.0) * (p3 - p0).dot((p1 - p0).cross(p2 - p0));

    Mat3d m;
    m.col(0) = p0 - p3;
    m.col(1) = p1 - p3;
    m.col(2) = p2 - p3;

    double det = m.determinant();
    if (fabs(det) > EPS)
    {
        m_invRestMat = m.inverse();
        return true;
    }

    return false;
}

bool
FEMTetConstraint::solvePositionConstraint(PositionBasedModel &model)
{
    const unsigned int i1 = m_bodies[0];
    const unsigned int i2 = m_bodies[1];
    const unsigned int i3 = m_bodies[2];
    const unsigned int i4 = m_bodies[3];

    auto state = model.getState();

    Vec3d &p0 = state->getVertexPosition(i1);
    Vec3d &p1 = state->getVertexPosition(i2);
    Vec3d &p2 = state->getVertexPosition(i3);
    Vec3d &p3 = state->getVertexPosition(i4);

    double currentVolume = (1.0 / 6.0) * (p3 - p0).dot((p1 - p0).cross(p2 - p0));

    Mat3d m;
    m.col(0) = p0 - p3;
    m.col(1) = p1 - p3;
    m.col(2) = p2 - p3;

    // deformation gradient
    Mat3d F = m*m_invRestMat;
    // First Piola-Kirchhoff tensor
    Mat3d P;
    // energy constraint
    double C = 0;

    double mu = model.getFirstLame();
    double lambda = model.getSecondLame();

    switch (m_material)
    {

    // P(F) = F*(2*mu*E + lambda*tr(E)*I)
    // E = (F^T*F - I)/2
    case MaterialType::StVK :
    {
        Mat3d E;
        E(0, 0) = 0.5*(F(0, 0) * F(0, 0) + F(1, 0) * F(1, 0) + F(2, 0) * F(2, 0) - 1.0);    // xx
        E(1, 1) = 0.5*(F(0, 1) * F(0, 1) + F(1, 1) * F(1, 1) + F(2, 1) * F(2, 1) - 1.0);    // yy
        E(2, 2) = 0.5*(F(0, 2) * F(0, 2) + F(1, 2) * F(1, 2) + F(2, 2) * F(2, 2) - 1.0);    // zz
        E(0, 1) = 0.5*(F(0, 0) * F(0, 1) + F(1, 0) * F(1, 1) + F(2, 0) * F(2, 1));          // xy
        E(0, 2) = 0.5*(F(0, 0) * F(0, 2) + F(1, 0) * F(1, 2) + F(2, 0) * F(2, 2));          // xz
        E(1, 2) = 0.5*(F(0, 1) * F(0, 2) + F(1, 1) * F(1, 2) + F(2, 1) * F(2, 2));          // yz
        E(1, 0) = E(0, 1);
        E(2, 0) = E(0, 2);
        E(2, 1) = E(1, 2);

        P = 2*mu*E;
        double tr = E.trace();
        double lt = lambda*tr;
        P(0,0) += lt;
        P(1,1) += lt;
        P(2,2) += lt;
        P = F*P;

        C = E(0,0)*E(0,0) + E(0,1)*E(0,1) + E(0,2)*E(0,2)
           + E(1,0)*E(1,0) + E(1,1)*E(1,1) + E(1,2)*E(1,2)
           + E(2,0)*E(2,0) + E(2,1)*E(2,1) + E(2,2)*E(2,2);
        C = mu*C + 0.5*lambda*tr*tr;

        break;
    }

    // P(F) = (2*mu*(F-R) + lambda*(J-1)*J*F^-T
    case MaterialType::Corotation :
    {
        Eigen::JacobiSVD<Mat3d> svd(F, Eigen::ComputeFullU | Eigen::ComputeFullV);
        Mat3d R = svd.matrixU()*svd.matrixV().adjoint();
        Vec3d Sigma(svd.singularValues());
        Mat3d invFT = svd.matrixU();
        invFT.col(0) /= Sigma(0);
        invFT.col(1) /= Sigma(1);
        invFT.col(2) /= Sigma(2);
        invFT *= svd.matrixV().adjoint();
        double J = Sigma(0)*Sigma(1)*Sigma(2);
        Mat3d FR = F - R;

        P = 2*mu*FR + lambda*(J-1)*J*invFT;

        C =  FR(0,0)*FR(0,0) + FR(0,1)*FR(0,1) + FR(0,2)*FR(0,2)
                + FR(1,0)*FR(1,0) + FR(1,1)*FR(1,1) + FR(1,2)*FR(1,2)
                + FR(2,0)*FR(2,0) + FR(2,1)*FR(2,1) + FR(2,2)*FR(2,2);
        C = mu*C + 0.5*lambda*(J-1)*(J-1);

        break;
    }
    // P(F) = mu*(F - mu*F^-T) + lambda*log(J)F^-T;
    case MaterialType::NeoHookean :
    {
        Mat3d invFT = F.inverse().transpose();
        double logJ = log(F.determinant());
        P = mu*(F - invFT) + lambda*logJ*invFT;

        C = F(0,0)*F(0,0) + F(0,1)*F(0,1) + F(0,2)*F(0,2)
                + F(1,0)*F(1,0) + F(1,1)*F(1,1) + F(1,2)*F(1,2)
                + F(2,0)*F(2,0) + F(2,1)*F(2,1) + F(2,2)*F(2,2);

        C = 0.5*mu*(C-3) - mu*logJ + 0.5*lambda*logJ*logJ;

        break;
    }

    case MaterialType::Linear :
    {
        break;
    }

    default: {
        printf("Material type not supported ! \n") ;
        break;
    }
    }

    const double im1 = state->getInvMass(i1);
    const double im2 = state->getInvMass(i2);
    const double im3 = state->getInvMass(i3);
    const double im4 = state->getInvMass(i4);

    Mat3d gradC = m_Volume*P*m_invRestMat.transpose();

    double sum = im1*gradC.col(0).squaredNorm()
            + im2*gradC.col(1).squaredNorm()
            + im3*gradC.col(2).squaredNorm()
            + im4*(gradC.col(0) + gradC.col(1) + gradC.col(2)).squaredNorm();

    if (sum < EPS)
        return false;

    C = C*m_Volume;

    double s = C/sum;

    if (im1 > 0)
        p0 += -s*im1*gradC.col(0);
    if (im2 > 0)
        p1 += -s*im2*gradC.col(1);
    if (im3 > 0)
        p2 += -s*im3*gradC.col(2);
    if (im4 > 0)
        p3 += s*im4*(gradC.col(0) + gradC.col(1) + gradC.col(2));

    return true;
}

void
DistanceConstraint::initConstraint(PositionBasedModel &model, const unsigned int &pIdx1,
                                   const unsigned int &pIdx2, const double k)
{
    m_bodies[0] = pIdx1;
    m_bodies[1] = pIdx2;
    m_stiffness = k;
    auto state = model.getState();
    Vec3d &p1 = state->getInitialVertexPosition(pIdx1);
    Vec3d &p2 = state->getInitialVertexPosition(pIdx2);
    m_restLength = (p1 - p2).norm();
}

bool
DistanceConstraint::solvePositionConstraint(PositionBasedModel &model)
{
    const unsigned int i1 = m_bodies[0];
    const unsigned int i2 = m_bodies[1];

    auto state = model.getState();

    Vec3d &p0 = state->getVertexPosition(i1);
    Vec3d &p1 = state->getVertexPosition(i2);

    const double im1 = state->getInvMass(i1);
    const double im2 = state->getInvMass(i2);

    double wsum = im1 + im2;

    if (wsum == 0.0)
        return false;
    Vec3d n = p1 - p0;
    double len = n.norm();
    n /= len;

    Vec3d gradC = m_stiffness*n*(len - m_restLength)/wsum;

    if (im1 > 0)
        p0 += im1*gradC;
    if (im2 > 0)
        p1 += -im2*gradC;

    return true;
}

void
DihedralConstraint::initConstraint(PositionBasedModel &model,
                                   const unsigned int &pIdx1, const unsigned int &pIdx2,
                                   const unsigned int &pIdx3, const unsigned int &pIdx4,
                                   const double k)
{
    m_bodies[0] = pIdx1;
    m_bodies[1] = pIdx2;
    m_bodies[2] = pIdx3;
    m_bodies[3] = pIdx4;

    m_stiffness = k;
    auto state = model.getState();

    Vec3d &p0 = state->getInitialVertexPosition(pIdx1);
    Vec3d &p1 = state->getInitialVertexPosition(pIdx2);
    Vec3d &p2 = state->getInitialVertexPosition(pIdx3);
    Vec3d &p3 = state->getInitialVertexPosition(pIdx4);

    Vec3d n1 = (p2 - p0).cross(p3 - p0).normalized();
    Vec3d n2 = (p3 - p1).cross(p2 - p1).normalized();

    m_restAngle = atan2(n1.cross(n2).dot(p3-p2), (p3-p2).norm()*n1.dot(n2));
}

bool
DihedralConstraint::solvePositionConstraint(PositionBasedModel &model)
{
    const unsigned int i1 = m_bodies[0];
    const unsigned int i2 = m_bodies[1];
    const unsigned int i3 = m_bodies[2];
    const unsigned int i4 = m_bodies[3];

    auto state = model.getState();

    Vec3d &p0 = state->getVertexPosition(i1);
    Vec3d &p1 = state->getVertexPosition(i2);
    Vec3d &p2 = state->getVertexPosition(i3);
    Vec3d &p3 = state->getVertexPosition(i4);

    const double im0 = state->getInvMass(i1);
    const double im1 = state->getInvMass(i2);
    const double im2 = state->getInvMass(i3);
    const double im3 = state->getInvMass(i4);

    if (im0 == 0.0 && im1 == 0.0)
        return false;
    Vec3d e = p3 - p2;
    Vec3d e1 = p3 - p0;
    Vec3d e2 = p0 - p2;
    Vec3d e3 = p3 - p1;
    Vec3d e4 = p1 - p2;
    // To accelerate, all normal (area) vectors and edge length should be precomputed in parallel
    Vec3d n1 = e1.cross(e);
    Vec3d n2 = e.cross(e3);
    double A1 = n1.norm();
    double A2 = n2.norm();
    n1 /= A1;
    n2 /= A2;

    double l = e.norm();
    if ( l < EPS )
        return false;

    Vec3d grad0 = -(l/A1)*n1;
    Vec3d grad1 = -(l/A2)*n2;
    Vec3d grad2 = (e.dot(e1)/(A1*l))*n1 + (e.dot(e3)/(A2*l))*n2 ;
    Vec3d grad3 = (e.dot(e2)/(A1*l))*n1 + (e.dot(e4)/(A2*l))*n2 ;

    double lambda = im0*grad0.squaredNorm() +
                    im1*grad1.squaredNorm() +
                    im2*grad2.squaredNorm() +
                    im3*grad3.squaredNorm();

    // huge difference if use acos instead of atan2
    lambda = (atan2(n1.cross(n2).dot(e), l*n1.dot(n2)) - m_restAngle) / lambda * m_stiffness;

    if (im0 > 0)
        p0 += -im0*lambda*grad0;
    if (im1 > 0)
        p1 += -im1*lambda*grad1;
    if (im2 > 0)
        p2 += -im2*lambda*grad2;
    if (im3 > 0)
        p3 += -im3*lambda*grad3;

    return true;
}

void
AreaConstraint::initConstraint(PositionBasedModel &model, const unsigned int &pIdx1,
                               const unsigned int &pIdx2, const unsigned int &pIdx3,
                               const double k)
{
    m_bodies[0] = pIdx1;
    m_bodies[1] = pIdx2;
    m_bodies[2] = pIdx3;

    m_stiffness = k;

    auto state = model.getState();

    Vec3d &p0 = state->getInitialVertexPosition(pIdx1);
    Vec3d &p1 = state->getInitialVertexPosition(pIdx2);
    Vec3d &p2 = state->getInitialVertexPosition(pIdx3);

    m_restArea = 0.5*(p1 - p0).cross(p2 - p0).norm();
}

bool
AreaConstraint::solvePositionConstraint(PositionBasedModel &model)
{
    const unsigned int i1 = m_bodies[0];
    const unsigned int i2 = m_bodies[1];
    const unsigned int i3 = m_bodies[2];

    auto state = model.getState();

    Vec3d &p0 = state->getVertexPosition(i1);
    Vec3d &p1 = state->getVertexPosition(i2);
    Vec3d &p2 = state->getVertexPosition(i3);

    const double im0 = state->getInvMass(i1);
    const double im1 = state->getInvMass(i2);
    const double im2 = state->getInvMass(i3);

    Vec3d e1 = p0 - p1;
    Vec3d e2 = p1 - p2;
    Vec3d e3 = p2 - p0;

    Vec3d n = e1.cross(e2);
    double A = 0.5*n.norm();

    if (A < EPS)
        return false;

    n /= 2*A;

    Vec3d grad0 = e2.cross(n);
    Vec3d grad1 = e3.cross(n);
    Vec3d grad2 = e1.cross(n);

    double lambda = im0*grad0.squaredNorm() + im1*grad1.squaredNorm() + im2*grad2.squaredNorm();

    lambda = (A - m_restArea) / lambda * m_stiffness;


    if (im0 > 0)
        p0 += -im0*lambda*grad0;
    if (im1 > 0)
        p1 += -im1*lambda*grad1;
    if (im2 > 0)
        p2 += -im2*lambda*grad2;

    return true;
}

void
VolumeConstraint::initConstraint(PositionBasedModel &model, const unsigned int &pIdx1,
                                 const unsigned int &pIdx2, const unsigned int &pIdx3,
                                 const unsigned int &pIdx4, const double k)
{
    m_bodies[0] = pIdx1;
    m_bodies[1] = pIdx2;
    m_bodies[2] = pIdx3;
    m_bodies[3] = pIdx4;

    m_stiffness = k;

    auto state = model.getState();

    Vec3d &p0 = state->getInitialVertexPosition(pIdx1);
    Vec3d &p1 = state->getInitialVertexPosition(pIdx2);
    Vec3d &p2 = state->getInitialVertexPosition(pIdx3);
    Vec3d &p3 = state->getInitialVertexPosition(pIdx4);

    m_restVolume = (1.0/6.0)*((p1-p0).cross(p2-p0)).dot(p3-p0);
}

bool
VolumeConstraint::solvePositionConstraint(PositionBasedModel &model)
{
    const unsigned int i1 = m_bodies[0];
    const unsigned int i2 = m_bodies[1];
    const unsigned int i3 = m_bodies[2];
    const unsigned int i4 = m_bodies[3];

    auto state = model.getState();

    Vec3d &x1 = state->getVertexPosition(i1);
    Vec3d &x2 = state->getVertexPosition(i2);
    Vec3d &x3 = state->getVertexPosition(i3);
    Vec3d &x4 = state->getVertexPosition(i4);

    const double im1 = state->getInvMass(i1);
    const double im2 = state->getInvMass(i2);
    const double im3 = state->getInvMass(i3);
    const double im4 = state->getInvMass(i4);

    double onesixth = 1.0/6.0;

    Vec3d grad1 = onesixth*(x2-x3).cross(x4-x2);
    Vec3d grad2 = onesixth*(x3-x1).cross(x4-x1);
    Vec3d grad3 = onesixth*(x4-x1).cross(x2-x1);
    Vec3d grad4 = onesixth*(x2-x1).cross(x3-x1);

    double V = grad4.dot(x4-x1);

    double lambda = im1*grad1.squaredNorm() +
                    im2*grad2.squaredNorm() +
                    im3*grad3.squaredNorm() +
                    im4*grad4.squaredNorm();

    lambda = (V - m_restVolume)/lambda * m_stiffness;

    if (im1 > 0)
        x1 += -im1*lambda*grad1;
    if (im1 > 0)
        x2 += -im2*lambda*grad2;
    if (im3 > 0)
        x3 += -im3*lambda*grad3;
    if (im4 > 0)
        x4 += -im4*lambda*grad4;

    return true;
}

} // imstk
