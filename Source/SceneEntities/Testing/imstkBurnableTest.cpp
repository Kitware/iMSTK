/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkBurnable.h"
#include "imstkCellMesh.h"
#include "imstkEntity.h"
#include "imstkLineMesh.h"
#include "imstkPbdMethod.h"
#include "imstkPbdSystem.h"
#include "imstkPbdModelConfig.h"

#include <gtest/gtest.h>

using namespace imstk;

TEST(imstkBurnableTest, testMemory)
{
    // Create line mesh
    auto                    lineMesh = std::make_shared<LineMesh>();
    VecDataArray<double, 3> vertices = { Vec3d(-0.5, 0.0, 0.0), Vec3d(0.0, 0.0, 0.0), Vec3d(0.5, 0.0, 0.0) };
    VecDataArray<int, 2>    indices  = { Vec2i(0, 1), Vec2i(1, 2) };
    lineMesh->initialize(
        std::make_shared<VecDataArray<double, 3>>(vertices),
        std::make_shared<VecDataArray<int, 2>>(indices));

    // Setup the PBD Model
    auto pbdSystem = std::make_shared<PbdSystem>();
    pbdSystem->getConfig()->m_doPartitioning = false;
    pbdSystem->getConfig()->m_dt = 0.005; // realtime used in update calls later in main
    pbdSystem->getConfig()->m_iterations = 4;
    pbdSystem->getConfig()->m_gravity    = Vec3d(0.0, -1.0, 0.0);
    pbdSystem->getConfig()->m_linearDampingCoeff  = 0.01; // Removed from velocity
    pbdSystem->getConfig()->m_angularDampingCoeff = 0.01;

    // Create Pbd object
    auto pbdObj = std::make_shared<Entity>();
    auto method = pbdObj->addComponent<PbdMethod>();
    method->setGeometry(lineMesh);
    method->setPbdSystem(pbdSystem);
    method->initialize();

    // Create burnable component
    auto burnable = std::make_shared<Burnable>();

    // Add component to pbd object
    pbdObj->addComponent(burnable);
    method->initialize();

    burnable->initialize();

    auto               burnStatePtr = std::dynamic_pointer_cast<DataArray<double>>(lineMesh->getCellAttribute("BurnDamage"));
    DataArray<double>& burnState    = *burnStatePtr;

    EXPECT_EQ(2, burnState.size()); // Should be number of cells
}