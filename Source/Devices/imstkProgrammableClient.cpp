/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkProgrammableClient.h"
#include "imstkLogger.h"
#include "imstkCapsule.h"

#include <iostream>

namespace imstk
{
ProgrammableClient::~ProgrammableClient()
{
    for (auto cmd : m_commands)
    {
        delete cmd;
    }
    m_commands.clear();
}

double
ProgrammableClient::getTotalDuration()
{
    double total = 0;
    for (auto cmd : m_commands)
    {
        total += cmd->duration;
    }
    return total;
}

void
ProgrammableClient::update()
{
    if (m_complete)
    {
        return;
    }
    if (m_dt <= 0)
    {
        LOG(WARNING) << "warning: No update period set";
        return;
    }

    size_t active   = 0;
    size_t complete = 0;
    m_currentTime += m_dt;
    for (auto cmd : m_commands)
    {
        if (cmd->state == CommandState::WAITING && m_currentTime > cmd->startTime)
        {
            cmd->activate(*this);
        }
        else if (cmd->state == CommandState::ACTIVE && m_currentTime >= cmd->startTime + cmd->duration)
        {
            cmd->complete(*this);
        }

        if (cmd->state == CommandState::ACTIVE)
        {
            active++;
            cmd->updateDevice(*this);
        }
        else if (cmd->state == CommandState::COMPLETE)
        {
            complete++;
        }
    }

    // if (active > 1)
    //   LOG(WARNING) << "More than 1 command is active for a Programmable Client";
    if (complete == m_commands.size())
    {
        m_complete = true;
    }
}

bool
ProgrammableClient::addLinearMovement(Vec3d startPos, Vec3d stopPos, double startTime, double duration)
{
    if (startPos == stopPos)
    {
        LOG(WARNING) << "warning: No travel set";
        return false;
    }
    if (duration <= 0)
    {
        LOG(WARNING) << "warning: No duration set";
        return false;
    }

    LinearMovement* move = new LinearMovement();
    move->startPosition = startPos;
    move->stopPosition  = stopPos;
    move->startTime     = startTime;
    move->duration      = duration;
    m_commands.push_back(move);
    return true;
}

void
ProgrammableClient::LinearMovement::activate(ProgrammableClient& pc)
{
    Command::activate(pc);
    pc.m_position = startPosition;
    pc.m_velocity = ((stopPosition - startPosition) / duration);
}

void
ProgrammableClient::LinearMovement::updateDevice(ProgrammableClient& pc)
{
    pc.m_position += (pc.getVelocity() * pc.m_dt);
}

void
ProgrammableClient::LinearMovement::complete(ProgrammableClient& pc)
{
    Command::complete(pc);
    pc.m_position = stopPosition;
}

bool
ProgrammableClient::addCircularMovement(Vec3d startPos, Vec3d centerPos, double startTime, double duration)
{
    if (duration <= 0)
    {
        LOG(WARNING) << "warning: No duration set";
        return false;
    }

    CircularMovement* move = new CircularMovement();
    move->startPosition  = startPos;
    move->centerPosition = centerPos;
    move->radius    = (startPos - centerPos).norm();
    move->angle     = acos((startPos[0] / move->radius) - centerPos[0]);
    move->angleStep = 2 * PI / (duration / m_dt);
    move->startTime = startTime;
    move->duration  = duration;
    m_commands.push_back(move);
    return true;
}

void
ProgrammableClient::CircularMovement::activate(ProgrammableClient& pc)
{
    Command::activate(pc);
    pc.m_position[0]     = centerPosition[0] + (cos(angle) * radius);
    pc.m_position[1]     = centerPosition[1];
    pc.m_position[2]     = centerPosition[2] + (sin(angle) * radius);
    pc.m_angularVelocity = Vec3d::Zero();
}

void
ProgrammableClient::CircularMovement::updateDevice(ProgrammableClient& pc)
{
    angle += angleStep;
    pc.m_position[0] = centerPosition[0] + (cos(angle) * radius);
    pc.m_position[1] = centerPosition[1];
    pc.m_position[2] = centerPosition[2] + (sin(angle) * radius);
    // TODO Compute angular velocity
}

void
ProgrammableClient::CircularMovement::complete(ProgrammableClient& pc)
{
    Command::complete(pc);
    pc.m_position[0]     = centerPosition[0] + (cos(0) * radius);
    pc.m_position[1]     = centerPosition[1];
    pc.m_position[2]     = centerPosition[2] + (sin(0) * radius);
    pc.m_angularVelocity = Vec3d::Zero();
}

bool
ProgrammableClient::addGrasping(std::shared_ptr<PbdObject> tool, std::shared_ptr<PbdObjectGrasping> objectGrasping, double startTime, double duration)
{
    GraspAction* grasp = new GraspAction();
    grasp->tool = tool;
    grasp->objectGrasping = objectGrasping;
    grasp->startTime      = startTime;
    grasp->duration       = duration;
    m_commands.push_back(grasp);
    return true;
}

void
ProgrammableClient::GraspAction::activate(ProgrammableClient& pc)
{
    Command::activate(pc);
    objectGrasping->beginVertexGrasp(std::dynamic_pointer_cast<Capsule>(tool->getPhysicsGeometry()));
}

void
ProgrammableClient::GraspAction::complete(ProgrammableClient& pc)
{
    Command::complete(pc);
    objectGrasping->endGrasp();
}

bool
ProgrammableClient::addLinearVertexMovement(
    std::shared_ptr<PbdObject> object,
    std::vector<int>           vertexIds,
    Vec3d                      translation,
    std::vector<bool>          pin,
    double                     startTime,
    double                     duration)
{
    if (translation == Vec3d::Zero())
    {
        LOG(WARNING) << "warning: No travel set";
    }
    if (duration <= 0)
    {
        LOG(WARNING) << "warning: No duration set";
        return false;
    }

    LinearVertexMovement* move = new LinearVertexMovement();
    move->object      = object;
    move->translation = translation;
    move->startTime   = startTime;
    move->duration    = duration;
    move->vertexIds   = vertexIds;
    move->pin = pin;

    if (move->vertexIds.empty())
    {
        LOG(WARNING) << "warning: Invalid initial position for vertex";
        return false;
    }

    m_commands.push_back(move);
    return true;
}

void
ProgrammableClient::LinearVertexMovement::activate(ProgrammableClient& pc)
{
    Command::activate(pc);
    pc.m_velocity = translation / duration;
    for (int i = 0; i < vertexIds.size(); i++)
    {
        currPos.push_back((*object->getPbdBody()->vertices)[vertexIds[i]]);
    }
}

void
ProgrammableClient::LinearVertexMovement::updateDevice(ProgrammableClient& pc)
{
    for (int i = 0; i < vertexIds.size(); i++)
    {
        for (int dim = 0; dim < 3; dim++)
        {
            if (pin[dim])
            {
                currPos[i][dim] += (pc.m_velocity[dim] * pc.m_dt);
            }
            else
            {
                currPos[i][dim] = (*object->getPbdBody()->vertices)[vertexIds[i]][dim];
            }
        }

        (*object->getPbdBody()->vertices)[vertexIds[i]]   = currPos[i];
        (*object->getPbdBody()->velocities)[vertexIds[i]] = Vec3d::Zero();// pc.m_velocity;
    }
}

void
ProgrammableClient::LinearVertexMovement::complete(ProgrammableClient& pc)
{
    Command::complete(pc);
}

bool
ProgrammableClient::addDeformation(
    std::shared_ptr<PbdObject> object,
    std::vector<int> vertexIds,
    double strain,
    DeformationType defType,
    double poisson,
    std::vector<bool> pin,
    double startTime, double duration)
{
    if (duration <= 0)
    {
        LOG(WARNING) << "warning: No duration set in addDeformation for ProgrammableClient";
        return false;
    }

    Deformation* deform = new Deformation();
    deform->object    = object;
    deform->strain    = strain;
    deform->startTime = startTime;
    deform->duration  = duration;
    deform->vertexIds = vertexIds;
    deform->pin       = pin;
    deform->type      = defType;
    deform->poissons  = poisson;

    if (deform->vertexIds.empty())
    {
        LOG(WARNING) << "warning: Invalid initial position for vertex";
        return false;
    }

    m_commands.push_back(deform);
    return true;
}

void
ProgrammableClient::Deformation::activate(ProgrammableClient& pc)
{
    Command::activate(pc);
    strainRate = strain / duration;
}

void
ProgrammableClient::Deformation::updateDevice(ProgrammableClient& pc)
{
    auto                                     mesh = std::dynamic_pointer_cast<PointSet>(object->getPhysicsGeometry());
    std::shared_ptr<VecDataArray<double, 3>> initVerticesPtr = mesh->getInitialVertexPositions();
    const VecDataArray<double, 3>&           initVertices    = *initVerticesPtr;

    std::shared_ptr<VecDataArray<double, 3>> verticesPtr = mesh->getVertexPositions();
    VecDataArray<double, 3>&                 vertices    = *verticesPtr;

    double volFac = poissons * 2.0;

    if (type == DeformationType::Compression)
    {
        double isoCompression = sqrt(1.0 / (1.0 - strainRate * pc.m_dt)) - 1.0;
        Mat3d  compression{
            { isoCompression* volFac, 0.0, 0.0 },
            { 0.0, -strainRate * pc.m_dt, 0.0 },
            { 0.0, 0.0, isoCompression* volFac } };

        defGrad += compression;
    }
    else if (type == DeformationType::Tension)
    {
        double isoTension = sqrt(1.0 / (1.0 + strainRate * pc.m_dt)) - 1.0;

        Mat3d tension{
            { isoTension* volFac, 0.0, 0.0 },
            { 0.0, strainRate* pc.m_dt, 0.0 },
            { 0.0, 0.0, isoTension* volFac } };

        defGrad += tension;
    }
    else if (type == DeformationType::SimpleShear)
    {
        double gamma = strainRate * pc.m_dt;
        Mat3d  simpleShear{
            { 0.0, gamma, 0.0 },
            { 0.0, 0.0, 0.0 },
            { 0.0, 0.0, 0.0 } };

        defGrad += simpleShear;
    }
    else if (type == DeformationType::PureShear)
    {
        double gamma = strainRate * pc.m_dt;
        Mat3d  pureShear{
            { 0.0, gamma, 0.0 },
            { gamma, 0.0, 0.0 },
            { 0.0, 0.0, 0.0 } };

        defGrad += pureShear;
    }

    for (int i = 0; i < vertexIds.size(); i++)
    {
        vertices[vertexIds[i]] = defGrad * initVertices[vertexIds[i]];
        (*object->getPbdBody()->vertices)[vertexIds[i]] = vertices[vertexIds[i]];
    }
}

void
ProgrammableClient::Deformation::complete(ProgrammableClient& pc)
{
    Command::complete(pc);
}

std::vector<int>
ProgrammableClient::findVertex(std::shared_ptr<PointSet> mesh, std::vector<Vec3d> initPos)
{
    std::vector<int> ids;
    for (Vec3d pos : initPos)
    {
        for (int i = 0; i < mesh->getNumVertices(); i++)
        {
            if (pos.isApprox(mesh->getInitialVertexPosition(i)))
            {
                ids.push_back(i);
            }
        }
    }

    return ids;
}

bool
ProgrammableClient::addWaitCommand(double startTime, double duration)
{
    if (duration <= 0)
    {
        LOG(WARNING) << "warning: No duration set";
        return false;
    }

    WaitCommand* wait = new WaitCommand();
    wait->startTime = startTime;
    wait->duration  = duration;
    m_commands.push_back(wait);
    return true;
}

bool
ProgrammableClient::addHoldCommand(std::shared_ptr<PbdObject> object, double startTime, double duration, std::vector<int> vertexIds)
{
    if (duration <= 0)
    {
        LOG(WARNING) << "warning: No duration set";
        return false;
    }

    if (vertexIds.size() <= 0)
    {
        LOG(WARNING) << "warning: Vertices to hold in addHoldCommand";
        return false;
    }

    HoldCommand* hold = new HoldCommand();
    hold->object    = object;
    hold->vertexIds = vertexIds;
    hold->startTime = startTime;
    hold->duration  = duration;

    m_commands.push_back(hold);
    return true;
}

void
ProgrammableClient::HoldCommand::activate(ProgrammableClient& pc)
{
    Command::activate(pc);

    // auto mesh = std::dynamic_pointer_cast<PointSet>(object->getPhysicsGeometry());
    if (object->getPhysicsGeometry()->isMesh())
    {
        auto mesh = std::dynamic_pointer_cast<PointSet>(object->getPhysicsGeometry());

        std::shared_ptr<VecDataArray<double, 3>> verticesPtr = mesh->getVertexPositions();
        VecDataArray<double, 3>&                 vertices    = *verticesPtr;

        for (int vert = 0; vert < vertexIds.size(); vert++)
        {
            // holdPosition.push_back((*object->getPbdBody()->vertices)[vertexIds[vert]]);
            //(*ptBody.invMasses)[0]
            auto body = object->getPbdBody();
            (*body->invMasses)[vertexIds[vert]] = 0.0;
            // (*(object->getPbdBody()).invMasses)[vertexIds[vert]] = 0.0;
            Vec3d pos = verticesPtr->at(vertexIds[vert]);
            holdPosition.push_back(pos);
        }
    }
    else
    {
        auto analyticalGeo = std::dynamic_pointer_cast<AnalyticalGeometry>(object->getPhysicsGeometry());
        auto body = object->getPbdBody();
        (*body->invMasses)[vertexIds[0]] = 0.0;
        // (*(object->getPbdBody()).invMasses)[vertexIds[vert]] = 0.0;
        Vec3d pos = analyticalGeo->getPosition();
        holdPosition.push_back(pos);
    }
}

void
ProgrammableClient::HoldCommand::updateDevice(ProgrammableClient& pc)
{
    if (object->getPhysicsGeometry()->isMesh())
    {
        auto mesh = std::dynamic_pointer_cast<PointSet>(object->getPhysicsGeometry());

        std::shared_ptr<VecDataArray<double, 3>> verticesPtr = mesh->getVertexPositions();
        VecDataArray<double, 3>&                 vertices    = *verticesPtr;

        for (int i = 0; i < vertexIds.size(); i++)
        {
            vertices[vertexIds[i]] = holdPosition[i];
            (*object->getPbdBody()->velocities)[vertexIds[i]] = Vec3d::Zero();
        }
    }
    else
    {
        auto analyticalGeo = std::dynamic_pointer_cast<AnalyticalGeometry>(object->getPhysicsGeometry());

        analyticalGeo->setPosition(holdPosition[0]);
        (*object->getPbdBody()->velocities)[vertexIds[0]] = Vec3d::Zero();
    }
}

void
ProgrammableClient::HoldCommand::complete(ProgrammableClient& pc)
{
    Command::complete(pc);
}

/*
  void ProgrammableClient::updateAngularOrientation()
  {
    // Quatd newOrientation = getOrientation();
    if (this->startPointSet)
    {

    std::cout << "Before:\n" << newOrientation << std::endl;
        newOrientation = newOrientation.slerp(0, Quatd(.707, 1, 1, 1));
        setOrientation(newOrientation);
        startPointSet = false;
    std::cout << "After:\n" << newOrientation << std::endl;
        return;
    }
    double temp = this->time / this->dt;

    std::cout << newOrientation << std::endl;
    Quatd temp = Quatd(.707, 0, 0, .707);
    setOrientation(temp);
  }*/
} // namespace imstk