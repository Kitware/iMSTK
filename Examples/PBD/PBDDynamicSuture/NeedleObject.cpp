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
    // setVisualGeometry(sutureLineMesh);
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
}