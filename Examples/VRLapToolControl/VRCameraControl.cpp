/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "VRCameraControl.h"
#include "imstkCamera.h"
#include "imstkLogger.h"
#include "imstkOpenVRDeviceClient.h"

using namespace imstk;

void
VRCameraControl::printControls()
{
    LOG(INFO) << "VRCameraControl controls";
    LOG(INFO) << "----------------------------------------------------------------------";
    LOG(INFO) << " | Left Trackpad   - rotate view";
    LOG(INFO) << " | Right Trakcpad  - translate view";
    LOG(INFO) << "----------------------------------------------------------------------";
}

void
VRCameraControl::update(const double& dt)
{
    // We may switch cameras on the controller
    m_deltaTransform = Mat4d::Identity();
    if (m_camera == nullptr)
    {
        return;
    }

    if (m_rotateDevice != nullptr)
    {
        m_rotateDevice->update();

        const Vec2d& pos  = m_rotateDevice->getTrackpadPosition();
        const Mat4d& view = m_camera->getView();
        m_camera->setView(
            view * mat4dRotation(Rotd(-pos[0] * m_rotateSpeedScale * dt, Vec3d(0.0, 1.0, 0.0))));
    }
    if (m_translateDevice != nullptr)
    {
        m_translateDevice->update();

        const Vec2d& pos = m_translateDevice->getTrackpadPosition();

        double dy = 0.0;
        if (m_translateDevice->getButton(2))
        {
            dy = m_translateVerticalSpeedScale;
        }
        if (m_translateDevice->getButton(3))
        {
            dy = -m_translateVerticalSpeedScale;
        }

        // Final view applied to scene
        const Mat4d& finalView = m_camera->getHMDView();
        // User view (multiplied into with hardware view)
        const Mat4d& userView = m_camera->getView();

        //Vec3d translate = Vec3d::Zero();
        //Mat3d rotation = Mat3d::Identity();
        //Vec3d scale = Vec3d::Ones();
        //mat4dTRS(finalView, translate, rotation, scale);

        //// If we want direction in world space we need the inverse
        //rotation = rotation.inverse();

        const Mat4d inverseFinalView = finalView.inverse();

        static int counter = 0;
        if (counter % 500 == 0)
        {
            //printf("x: %f, %f, %f\n", rotation.col(0)[0], rotation.col(0)[1], rotation.col(0)[2]);
            //printf("y: %f, %f, %f\n", rotation.col(1)[0], rotation.col(1)[1], rotation.col(1)[2]);
            const Vec3d viewDir = inverseFinalView.col(2).head<3>();
            printf("z: %f, %f, %f\n", viewDir[0], viewDir[1], viewDir[2]);
        }
        counter++;

        /*Matrix3f n;
        n = AngleAxisf(ea[0], Vector3f::UnitX())
            * AngleAxisf(ea[1], Vector3f::UnitY())
            * AngleAxisf(ea[2], Vector3f::UnitZ());*/

        //
        // View directions/basis in our world space
        Vec3d       worldViewXDir = inverseFinalView.col(0).head<3>().normalized();
        const Vec3d yDir = Vec3d(0.0, 1.0, 0.0);
        Vec3d       worldViewZDir = inverseFinalView.col(2).head<3>().normalized();

        // Orthognalize into world space
        // ie: Project z and x onto yDir plane
        //xDir = zDir.cross(yDir);
        //zDir = xDir.cross(yDir);

        const Vec3d movement = worldViewXDir * -pos[0] + worldViewZDir * pos[1] + yDir * dy;
        m_deltaTransform = mat4dTranslate(movement * m_translateSpeedScale * dt);
        m_camera->setView(userView * m_deltaTransform);
    }
}