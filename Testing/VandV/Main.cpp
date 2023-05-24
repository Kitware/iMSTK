/* This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details. */

#include "ProgrammableSceneExec.h"

#include "Scenes/BendingBeamScene.h"
#include "Scenes/CapsuleToCube.h"
#include "Scenes/ChainDropScene.h"
#include "Scenes/LoadableMeshScene.h"
#include "Scenes/SpringMassDamperScene.h"
#include "Scenes/UnitCubeStrainScene.h"

using namespace imstk;

bool CMBBE2023(ProgrammableSceneExec& exec);

int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    ProgrammableSceneExec exec;
    exec.m_viewScene = false;

    bool bRet = true;

    // bRet &= CMBBE2023(exec);

    // CapsuleToCubeScene::Configuration c2cConfig;
    // c2cConfig.type = CapsuleToCubeScene::Type::DeformableCube;
    // c2cConfig.sceneOutputDirectory = "CapsuleToCube/DeformableCube/";
    // bRet &= exec.executeScene(std::make_shared<CapsuleToCubeScene>(c2cConfig));

    // c2cConfig.divisions = Vec3i(18, 4, 18);
    // c2cConfig.sceneOutputDirectory = "CapsuleToCube/HighResDeformableCube/";
    // bRet &= exec.executeScene(std::make_shared<CapsuleToCubeScene>(c2cConfig));

    // c2cConfig.divisions = Vec3i(3, 3, 3);
    // c2cConfig.type      = CapsuleToCubeScene::Type::RigidCube;
    // c2cConfig.sceneOutputDirectory = "CapsuleToCube/RigidCube/";
    // bRet &= exec.executeScene(std::make_shared<CapsuleToCubeScene>(c2cConfig));

    // c2cConfig.type = CapsuleToCubeScene::Type::GraspDeformableCube;
    // c2cConfig.sceneOutputDirectory = "CapsuleToCube/GraspDeformableCube/";
    // bRet &= exec.executeScene(std::make_shared<CapsuleToCubeScene>(c2cConfig));

    // LoadableMeshScene::Configuration meshConfig;
    // meshConfig.filePath = std::string(iMSTK_DATA_ROOT) + std::string("/Organs/Kidney/kidney_vol_low_rez.vtk");
    // meshConfig.mass     = 60;
    // bRet &= exec.executeScene(std::make_shared<LoadableMeshScene>(meshConfig));

    exec.report();

    return !bRet;
}
