/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkDeviceClient.h"
#include "imstkPbdObject.h"
#include "imstkPbdObjectGrasping.h"
#include "imstkSurfaceMesh.h"

#include <unordered_map>

namespace imstk
{
///
/// \class ProgrammableClient
///
/// \brief Allows setting the pose of the device from external caller without a
///  real device connected
///
class ProgrammableClient : public DeviceClient
{
public:
    ///
    /// \brief Constructor
    ///
    ProgrammableClient(const std::string& name = "") : DeviceClient(name, "localhost") {}

    ///
    /// \brief Destructor
    ///
    ~ProgrammableClient() override;

    ///
    /// \brief Enum for type of deformation
    ///
    enum  DeformationType { Compression, Tension, SimpleShear, PureShear };
protected:
    void run()     = delete;
    void cleanUp() = delete;

    ///
    /// \brief Enum for current state of command
    ///
    enum class CommandState { WAITING=0, ACTIVE, COMPLETE };

    ///
    /// \brief General struct for command. Command will call activate when start time is reached,
    ///        call updateDevice while active, and call complete when duration is over.
    ///
    struct Command
    {
        virtual ~Command() = default;
        CommandState state = CommandState::WAITING;
        double startTime   = 0.0;
        double duration    = 0.0;
        virtual void activate(ProgrammableClient&) { state = CommandState::ACTIVE; }
        virtual void updateDevice(ProgrammableClient&) = 0;
        virtual void complete(ProgrammableClient&) { state = CommandState::COMPLETE; }
    };
    friend Command;

    ///
    /// \brief Command for linear movement of an analytical geometry
    ///
    struct LinearMovement : Command
    {
        Vec3d startPosition = Vec3d::Zero();
        Vec3d stopPosition  = Vec3d::Zero();
        void activate(ProgrammableClient&) override;
        void updateDevice(ProgrammableClient&) override;
        void complete(ProgrammableClient&) override;
    };

    ///
    /// \brief Command for circular movement of an analytical geometry
    ///
    struct CircularMovement : Command
    {
        Vec3d startPosition  = Vec3d::Zero();
        Vec3d centerPosition = Vec3d::Zero();
        double radius    = 0.0;
        double angle     = 0;
        double angleStep = 0;
        void activate(ProgrammableClient&) override;
        void updateDevice(ProgrammableClient&) override;
        void complete(ProgrammableClient&) override;
    };

    ///
    /// \brief Command for grasping an object
    ///
    struct GraspAction : Command
    {
        std::shared_ptr<PbdObjectGrasping> objectGrasping;
        std::shared_ptr<PbdObject> tool;
        void activate(ProgrammableClient&) override;
        void updateDevice(ProgrammableClient&) override { return; }
        void complete(ProgrammableClient&) override;
    };

    ///
    /// \brief Command for linear movement of a subset of vertices from an object
    ///
    struct LinearVertexMovement : Command
    {
        std::shared_ptr<PbdObject> object;
        Vec3d translation = Vec3d::Zero();
        std::vector<bool> pin{ true, true, true };
        std::vector<int> vertexIds;
        std::vector<Vec3d> currPos;
        void activate(ProgrammableClient&) override;
        void updateDevice(ProgrammableClient&) override;
        void complete(ProgrammableClient&) override;
    };

    ///
    /// \brief Command for applying deformationt to a subset of vertices from an object
    ///
    struct Deformation : Command
    {
        std::shared_ptr<PbdObject> object;
        Mat3d defGrad     = Mat3d::Identity();
        double strain     = 0.0;
        double strainRate = 0.0;
        double poissons   = 0.49;
        std::vector<bool> pin{ true, true, true };
        std::vector<int> vertexIds;

        DeformationType type;

        void activate(ProgrammableClient&) override;
        void updateDevice(ProgrammableClient&) override;
        void complete(ProgrammableClient&) override;
    };

    ///
    /// \brief Command for waiting. Used to let system continue running with no active commands
    ///
    struct WaitCommand : Command
    {
        void activate(ProgrammableClient& pc) override { Command::activate(pc); }
        void updateDevice(ProgrammableClient&) override { return; }
        void complete(ProgrammableClient& pc) override { Command::complete(pc); }
    };

    ///
    /// \brief Command for holding a subset of vertices at a specific position
    ///
    struct HoldCommand : Command
    {
        std::shared_ptr<PbdObject> object;
        std::vector<int> vertexIds;
        std::vector<Vec3d> holdPosition;
        void activate(ProgrammableClient& pc) override;
        void updateDevice(ProgrammableClient&) override;
        void complete(ProgrammableClient& pc) override;
    };

    double m_dt = 0.0;
    double m_currentTime = 0.0;
    bool   m_complete    = false;
    std::vector<Command*> m_commands;

public:

    ///
    /// \brief Set the dt of the system. Should be the same as the simulation dt
    ///
    void setDeltaTime(double dt) { m_dt = dt; }

    ///
    /// \brief Update all commands
    ///
    void update() override;

    ///
    /// \brief Add a linear movement command to run
    /// \param startPos starting position for movement
    /// \param stopPos ending position for movement
    /// \param startTime time when command should activate
    /// \param duration time when command should be active
    ///
    bool addLinearMovement(Vec3d startPos, Vec3d stopPos, double startTime, double duration);

    ///
    /// \brief Add a circular movement command to run
    /// \param startPos starting position for movement
    /// \param centerPos center of circle to move around
    /// \param startTime time when command should activate
    /// \param duration time when command should be active
    ///
    bool addCircularMovement(Vec3d startPos, Vec3d centerPos, double startTime, double duration);

    ///
    /// \brief Add a grasp command to run
    /// \param tool pbdObject the perfroms the grasping
    /// \param objectGrasping how grasping action is defined
    /// \param startTime time when command should activate
    /// \param duration time when command should be active
    ///
    bool addGrasping(std::shared_ptr<PbdObject> tool, std::shared_ptr<PbdObjectGrasping> objectGrasping, double startTime, double duration);

    ///
    /// \brief Add a linear vertex movement command to run
    /// \param object pbdObject that contains vertices to move
    /// \param vertexIds subset of vertices to move
    /// \param translation vector of distance to move
    /// \param pin vecotr of booleans to control which dims to move
    /// \param startTime time when command should activate
    /// \param duration time when command should be active
    ///
    bool addLinearVertexMovement(std::shared_ptr<PbdObject> object, std::vector<int> vertexIds, Vec3d translation, std::vector<bool> pin, double startTime, double duration);

    ///
    /// \brief Add a defomation command to run
    /// \param object pbdObject that contains vertices to move
    /// \param initPos subset of vertex's initial positions to move
    /// \param strain strain of deofmration
    /// \param DeformationType type of deforamtion to apply
    /// \param poisson
    /// \param pin vecotr of bools to control which dims to move
    /// \param startTime time when command should activate
    /// \param duration time when command should be active
    ///
    bool addDeformation(std::shared_ptr<PbdObject> object, std::vector<int> initPos, double strain, DeformationType defType, double poisson, std::vector<bool> pin, double startTime, double duration);

    // TODO remove once addDeformation has been updated
    std::vector<int> findVertex(std::shared_ptr<PointSet> mesh, std::vector<Vec3d> initPos);

    ///
    /// \brief Add a wait command to run
    /// \param startTime time when command should activate
    /// \param duration time when command should be active
    ///
    bool addWaitCommand(double startTime, double duration);

    ///
    /// \brief Add a hold command to run
    /// \param object pbdObject that contains vertices to hold
    /// \param startTime time when command should activate
    /// \param duration time when command should be active
    /// \param vertexIds vecotr of vertex ids to hold
    ///
    bool addHoldCommand(std::shared_ptr<PbdObject> object, double startTime, double duration, std::vector<int> vertexIds);

    ///
    /// \brief Returns to total duration of all commands
    ///
    double getTotalDuration();

    ///
    /// \brief Returns true if all commands have finished executing. Returns false otherwise
    ///
    bool isFinished() { return this->m_complete; }

    void setOrientation(Quatd temp) { m_orientation = temp; }
};
} // namespace imstk
