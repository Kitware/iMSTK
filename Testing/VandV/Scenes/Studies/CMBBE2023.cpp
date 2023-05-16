/* This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details. */

#include "ProgrammableSceneExec.h"

#include "Scenes/BendingBeamScene.h"
#include "Scenes/CapsuleToCube.h"
#include "Scenes/ChainDropScene.h"
#include "Scenes/SpringMassDamperScene.h"
#include "Scenes/UnitCubeStrainScene.h"

using namespace imstk;

bool unitCubeStudyTension(ProgrammableSceneExec&);
bool unitCubeStudyCompression(ProgrammableSceneExec&);
bool unitCubeStudyShear(ProgrammableSceneExec&);
bool springMassStudy(ProgrammableSceneExec&);
bool bendingBeamStudy(ProgrammableSceneExec&);
bool chainDropStudy(ProgrammableSceneExec&);

bool
CMBBE2023(ProgrammableSceneExec& exec)
{
    bool bRet = true;
    // bRet &= chainDropStudy(exec);
    // bRet &= bendingBeamStudy(exec);
    // bRet &= springMassStudy(exec);
    // bRet &= unitCubeStudyCompression(exec);
    // bRet &= unitCubeStudyShear(exec);
    // bRet &= unitCubeStudyTension(exec);

    return bRet;
}

bool
unitCubeStudyTension(ProgrammableSceneExec& exec)
{
    std::vector<int>   iterations{ 1, 3, 5 };
    std::vector<Vec3i> partitions{
        { 4, 4, 4 },
        { 6, 6, 6 },
        { 8, 8, 8 }, };

    for (size_t itr = 0; itr < iterations.size(); itr++)
    {
        for (size_t refine = 0; refine < partitions.size(); refine++)
        {
            UnitCubeStrainScene::Configuration cfg;
            cfg.iterations = iterations[itr];
            cfg.partitions = partitions[refine];
            cfg.strain     = 0.20;
            cfg.compressionDuration = 5;
            cfg.youngsModulus       = 1000.0; // https://doi.org/10.1016/S0301-5629(02)00489-1
            cfg.poissonRatio         = 0.49;  // https://doi.org/10.1118/1.279566
            cfg.deformationType      = ProgrammableClient::DeformationType::Tension;
            cfg.materialType         = PbdFemConstraint::MaterialType::StVK;
            cfg.sceneOutputDirectory = "cmbbe2023/UnitCubeStrain/Tension_" + std::to_string(cfg.iterations) + "_Iterations_" + std::to_string(cfg.partitions[0]) + "_Partitions/";

            if (!exec.executeScene(std::make_shared<UnitCubeStrainScene>(cfg)))
            {
                return false;
            }
        }
    }
    return true;
}

bool
unitCubeStudyCompression(ProgrammableSceneExec& exec)
{
    std::vector<int>   iterations{ 1, 3, 5 };
    std::vector<Vec3i> partitions{
        { 4, 4, 4 },
        { 6, 6, 6 },
        { 8, 8, 8 }, };

    for (size_t itr = 0; itr < iterations.size(); itr++)
    {
        for (size_t refine = 0; refine < partitions.size(); refine++)
        {
            UnitCubeStrainScene::Configuration cfg;
            cfg.iterations = iterations[itr];
            cfg.partitions = partitions[refine];
            cfg.strain     = 0.20;
            cfg.compressionDuration = 5;
            cfg.youngsModulus       = 1000.0; // https://doi.org/10.1016/S0301-5629(02)00489-1
            cfg.poissonRatio         = 0.49;  // https://doi.org/10.1118/1.279566
            cfg.deformationType      = ProgrammableClient::DeformationType::Compression;
            cfg.materialType         = PbdFemConstraint::MaterialType::StVK;
            cfg.sceneOutputDirectory = "cmbbe2023/UnitCubeStrain/Compression_" + std::to_string(cfg.iterations) + "_Iterations_" + std::to_string(cfg.partitions[0]) + "_Partitions/";

            if (!exec.executeScene(std::make_shared<UnitCubeStrainScene>(cfg)))
            {
                return false;
            }
        }
    }
    return true;
}

bool
unitCubeStudyShear(ProgrammableSceneExec& exec)
{
    std::vector<int>   iterations{ 1, 3, 5 };
    std::vector<Vec3i> partitions{
        { 4, 4, 4 },
        { 6, 6, 6 },
        { 8, 8, 8 }, };

    for (size_t itr = 0; itr < iterations.size(); itr++)
    {
        for (size_t refine = 0; refine < partitions.size(); refine++)
        {
            UnitCubeStrainScene::Configuration cfg;
            cfg.iterations = iterations[itr];
            cfg.partitions = partitions[refine];
            cfg.strain     = 0.20;
            cfg.compressionDuration = 5;
            cfg.youngsModulus       = 1000.0; // https://doi.org/10.1016/S0301-5629(02)00489-1
            cfg.poissonRatio         = 0.49;  // https://doi.org/10.1118/1.279566
            cfg.deformationType      = ProgrammableClient::DeformationType::PureShear;
            cfg.materialType         = PbdFemConstraint::MaterialType::StVK;
            cfg.sceneOutputDirectory = "cmbbe2023/UnitCubeStrain/Shear_" + std::to_string(cfg.iterations) + "_Iterations_" + std::to_string(cfg.partitions[0]) + "_Partitions/";

            if (!exec.executeScene(std::make_shared<UnitCubeStrainScene>(cfg)))
            {
                return false;
            }
        }
    }
    return true;
}

bool
springMassStudy(ProgrammableSceneExec& exec)
{
    std::vector<int> iterations{ 1, 10, 100, 1000 };

    for (size_t itr = 0; itr < iterations.size(); itr++)
    {
        SpringMassDamperScene::Configuration cfg;
        cfg.iterations = iterations[itr];
        cfg.dt = 0.001;
        cfg.elongationPercent = 20;
        cfg.mass      = 1.0;
        cfg.stiffness = 10.0;
        cfg.sceneOutputDirectory = "cmbbe2023/SpringMassDamper/" + std::to_string(cfg.iterations) + "Iterations/";

        if (!exec.executeScene(std::make_shared<SpringMassDamperScene>(cfg)))
        {
            return false;
        }
    }
    return true;
}

bool
chainDropStudy(ProgrammableSceneExec& exec)
{
    std::vector<int> iterations{ 1, 10, 100, 1000 };

    for (size_t itr = 0; itr < iterations.size(); itr++)
    {
        ChainDropScene::Configuration cfg;
        cfg.iterations  = iterations[itr];
        cfg.chainLength = 20.0;
        cfg.mass      = 1.0;
        cfg.stiffness = 1E8;
        cfg.dt = 0.005;
        cfg.sceneOutputDirectory = "cmbbe2023/ChainDrop/" + std::to_string(cfg.iterations) + "Iterations/";

        if (!exec.executeScene(std::make_shared<ChainDropScene>(cfg)))
        {
            return false;
        }
    }
    return true;
}

bool
bendingBeamStudy(ProgrammableSceneExec& exec)
{
    std::vector<int>   iterations{ 1, 3, 5 };
    std::vector<Vec3i> partitions{
        { 15, 5, 3 },
        { 20, 5, 3 },
        { 40, 5, 3 },
    };
    std::map<BendingBeamScene::Configuration, CSVComparison> results;

    for (size_t itr = 0; itr < iterations.size(); itr++)
    {
        for (size_t refine = 0; refine < partitions.size(); refine++)
        {
            BendingBeamScene::Configuration cfg;
            cfg.iterations    = iterations[itr];
            cfg.displacement  = 1.0;
            cfg.youngsModulus = 1000.0;
            cfg.poissonRatio  = 0.49;
            cfg.partitions    = partitions[refine];
            cfg.materialType  = PbdFemConstraint::MaterialType::StVK;
            cfg.sceneOutputDirectory = "cmbbe2023/BendingBeam/" + std::to_string(cfg.iterations) + "_Iterations_" +
                                       std::to_string(cfg.partitions[0]) + "x" + std::to_string(cfg.partitions[1]) + "x" + std::to_string(cfg.partitions[2]) + "_Partitions/";

            if (!exec.executeScene(std::make_shared<BendingBeamScene>(cfg)))
            {
                return false;
            }
        }
    }
    return true;
}
