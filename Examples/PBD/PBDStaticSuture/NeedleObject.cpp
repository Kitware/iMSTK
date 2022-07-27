/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "NeedleObject.h"
#include "imstkIsometricMap.h"
#include "imstkLineMesh.h"
#include "imstkMeshIO.h"
#include "imstkRbdConstraint.h"
#include "imstkRenderMaterial.h"
#include "imstkRigidBodyModel2.h"
#include "imstkSurfaceMesh.h"
#include "imstkVecDataArray.h"
#include "imstkVisualModel.h"

using namespace imstk;

NeedleObject::NeedleObject() : RigidObject2("Needle")
{
    auto sutureMesh     = MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/Surgical Instruments/Needles/c6_suture.stl");
    auto sutureLineMesh = MeshIO::read<LineMesh>(iMSTK_DATA_ROOT "/Surgical Instruments/Needles/c6_suture_hull.vtk");

    const Mat4d rot = mat4dRotation(Rotd(-PI_2, Vec3d(0.0, 1.0, 0.0))) *
                      mat4dRotation(Rotd(-0.6, Vec3d(1.0, 0.0, 0.0)));
    sutureMesh->transform(rot, Geometry::TransformType::ApplyToData);
    sutureLineMesh->transform(rot, Geometry::TransformType::ApplyToData);

    setVisualGeometry(sutureMesh);
    setCollidingGeometry(sutureLineMesh);
    setPhysicsGeometry(sutureLineMesh);
    setPhysicsToVisualMap(std::make_shared<IsometricMap>(sutureLineMesh, sutureMesh));
    getVisualModel(0)->getRenderMaterial()->setColor(Color(0.9, 0.9, 0.9));
    getVisualModel(0)->getRenderMaterial()->setShadingModel(RenderMaterial::ShadingModel::PBR);
    getVisualModel(0)->getRenderMaterial()->setRoughness(0.5);
    getVisualModel(0)->getRenderMaterial()->setMetalness(1.0);

    std::shared_ptr<RigidBodyModel2> rbdModel = std::make_shared<RigidBodyModel2>();
    rbdModel->getConfig()->m_gravity = Vec3d::Zero();
    rbdModel->getConfig()->m_maxNumIterations = 5;
    setDynamicalModel(rbdModel);

    getRigidBody()->m_mass = 1.0;
    getRigidBody()->m_intertiaTensor = Mat3d::Identity() * 10000.0;
    getRigidBody()->m_initPos = Vec3d(0.0, 0.0, 0.0);

    // Manually setup an arc aligned with the geometry, some sort of needle+arc generator
    // could be a nice addition to imstk
    Mat3d arcBasis = Mat3d::Identity();
    arcBasis.col(0) = Vec3d(0.0, 0.0, -1.0);
    arcBasis.col(1) = Vec3d(1.0, 0.0, 0.0);
    arcBasis.col(2) = Vec3d(0.0, 1.0, 0.0);
    arcBasis = rot.block<3, 3>(0, 0) * arcBasis;
    const Vec3d  arcCenter = (rot * Vec4d(0.0, -0.005455, 0.008839, 1.0)).head<3>();
    const double arcRadius = 0.010705;
    setArc(arcCenter, arcBasis, arcRadius, 0.558, 2.583);
}

const Mat3d
NeedleObject::getArcBasis()
{
    return getRigidBody()->getOrientation().toRotationMatrix() * m_arcBasis;
}

const Vec3d
NeedleObject::getArcCenter()
{
    return getRigidBody()->getPosition() + getRigidBody()->getOrientation().toRotationMatrix() * m_arcCenter;
}