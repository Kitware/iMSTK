/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkAnalyticalGeometry.h"
#include "imstkBurner.h"
#include "imstkCDObjectFactory.h"
#include "imstkCellMesh.h"
#include "imstkCellPicker.h"
#include "imstkParallelUtils.h"
#include "imstkPbdObject.h"
#include "imstkPbdModel.h"
#include "imstkPbdModelConfig.h"
#include "imstkPickingAlgorithm.h"
#include "imstkTaskGraph.h"
#include "imstkTaskNode.h"
#include "imstkParallelFor.h"

namespace imstk
{
Burner::Burner(const std::string& name) : SceneBehaviour(true, name)
{
    m_burningHandleNode = std::make_shared<TaskNode>([this]()
        {
            handle();
    }, "Handle_" + m_name);
}

void
Burner::init()
{
    m_burningObj = std::dynamic_pointer_cast<PbdObject>(getEntity().lock());

    CHECK(m_burningObj != nullptr) << "Burner requires an object to do the burning";

    if (m_burnGeometry == nullptr)
    {
        m_burnGeometry = std::dynamic_pointer_cast<AnalyticalGeometry>(m_burningObj->getPhysicsGeometry());
        CHECK(m_burnGeometry != nullptr) << "Burner requires analytical geometry for physics geometry";
    }

    // Verify that some objects are burnable
    if (m_burnableObjects.empty())
    {
        LOG(INFO) << "There are no burnable objects for the burning tool to burn.";
    }

    // Add task nodes
    m_taskGraph->addNode(m_burningHandleNode);
    m_taskGraph->addNode(m_burningObj->getPbdModel()->getTaskGraph()->getSink());
    m_taskGraph->addNode(m_burningObj->getPbdModel()->getIntegratePositionNode());
    m_taskGraph->addNode(m_burningObj->getPbdModel()->getSolveNode());
}

void Burner::visualUpdate(const double& dt)
{
    m_burnOnce = true;
    m_burnTime = dt;
}

void
Burner::handle()
{
    // Check tool state
    if (m_onState && m_burnOnce)
    {
        m_burnOnce = false;
        ParallelUtils::parallelFor(m_burnableObjects.size(), [this](const int index) {
            handleBurnable(index);
            }, m_burnableObjects.size() > 1);
    }
}

void
Burner::handleBurnable(int burnableId)
{
	if (m_burnableObjects[burnableId].object == nullptr)
	{
		return;
	}

	if (m_burnableObjects[burnableId].picker == nullptr)
	{
		// Create Picking Algorithms
		auto cellPicker = std::make_shared<CellPicker>();
		cellPicker->setPickingGeometry(m_burnGeometry);

		std::shared_ptr<Geometry> pbdPhysicsGeom = m_burnableObjects[burnableId].object->getPhysicsGeometry();
		CHECK(pbdPhysicsGeom != nullptr) << "Physics geometry of burnable object: " << m_burnableObjects[burnableId].object->getName() << " is null in Burner";

		auto cdType = CDObjectFactory::getCDType(*m_burnGeometry, *pbdPhysicsGeom);

		// TODO check if we can make the COllision remove burnable if not

		cellPicker->setCollisionDetection(CDObjectFactory::makeCollisionDetection(cdType));

		m_burnableObjects[burnableId].picker = cellPicker;
	}

	// Perform the picking on the burnable object
	std::shared_ptr<Geometry>    geometryToPick = m_burnableObjects[burnableId].object->getPhysicsGeometry();
	const std::vector<PickData>& pickData = m_burnableObjects[burnableId].picker->pick(geometryToPick);

	for (size_t i = 0; i < pickData.size(); i++)
	{
		const PickData& data = pickData[i];

		// If no cell is grabbed, go to next PickData
		if (data.cellId == -1)
		{
			continue;
		}

		// Integrate the burn state with time
		applyBurn(burnableId, data.cellId);
	}
}

void
Burner::applyBurn(int burnableId, int cellId)
{
    // Get model data
    // double dt = m_burnableObjects[burnableId].object->getPbdModel()->getConfig()->m_dt;

    // Mesh state data
    auto               cellMesh      = std::dynamic_pointer_cast<AbstractCellMesh>(m_burnableObjects[burnableId].object->getPhysicsGeometry());
    auto               burnDamagePtr = std::dynamic_pointer_cast<DataArray<double>>(cellMesh->getCellAttribute("BurnDamage"));
    DataArray<double>& burnDamage    = *burnDamagePtr;

    auto               burnVisualPtr = std::dynamic_pointer_cast<DataArray<double>>(cellMesh->getCellAttribute("BurnVisual"));
    DataArray<double>& burnVisual    = *burnVisualPtr;

    monopolarToolModel(burnDamage[cellId], burnVisual[cellId], m_burnTime);
}

void
Burner::monopolarToolModel(double& burnDmg, double& burnVis, double dt)
{
    burnDmg += m_onTime * m_q * m_normWattage * dt;
    burnVis += (1.0 - m_onTime) * m_q * m_normWattage * dt;
}

void
Burner::initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink)
{
    m_taskGraph->addEdge(source, m_burningObj->getPbdModel()->getIntegratePositionNode());
    m_taskGraph->addEdge(m_burningObj->getPbdModel()->getIntegratePositionNode(), m_burningHandleNode);
    m_taskGraph->addEdge(m_burningHandleNode, m_burningObj->getPbdModel()->getSolveNode());
    m_taskGraph->addEdge(m_burningObj->getPbdModel()->getSolveNode(), m_burningObj->getPbdModel()->getTaskGraph()->getSink());
    m_taskGraph->addEdge(m_burningObj->getPbdModel()->getTaskGraph()->getSink(), sink);
}
} // namespace imstk