/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "gtest/gtest.h"

#include "imstkTrackingDeviceControl.h"
#include "imstkDeviceClient.h"
#include "imstkMath.h"

using namespace imstk;

class MockDeviceClient : public DeviceClient
{
public:
    // Deviceclient has protected constructor ...
    MockDeviceClient() : DeviceClient("Mock", "") {}

    void setPosition(const Vec3d& position)
    {
        m_position = position;
    }

    void setOrientation(const Quatd& orientation)
    {
        m_orientation = orientation;
    }
};

// Derive to enable protected constructor
class MockTrackingDeviceControl : public TrackingDeviceControl
{
public:
    void update(const double&) override { }
};

class TrackingDeviceControlTest : public testing::Test
{
public:
    void SetUp() override
    {
        client = std::make_shared<MockDeviceClient>();
        control.setDevice(client);
    }

    MockTrackingDeviceControl control;
    std::shared_ptr<MockDeviceClient> client;
};

TEST_F(TrackingDeviceControlTest, Basics)
{
    control.setDevice(nullptr);
    EXPECT_FALSE(control.updateTrackingData(0.0));

    control.setDevice(client);
    EXPECT_TRUE(control.updateTrackingData(0.0));

    auto pos = Vec3d(1.0, 2.0, 3.0);
    auto rot = Quatd(4.0, 5.0, 6.0, 7.0).normalized();

    client->setPosition(pos);
    client->setOrientation(rot);

    control.updateTrackingData(0.0);

    EXPECT_TRUE(pos.isApprox(control.getPosition()));
    EXPECT_TRUE(rot.isApprox(control.getOrientation()));
}

TEST_F(TrackingDeviceControlTest, EndEffector)
{
    auto offset = Quatd(Eigen::AngleAxisd(PI * 0.5, Eigen::Vector3d::UnitY()));

    control.setEffectorRotationOffset(offset);

    auto pos = Vec3d(1.0, 2.0, 3.0);
    auto rot = Quatd(4.0, 5.0, 6.0, 7.0).normalized();

    client->setPosition(pos);
    client->setOrientation(rot);

    control.updateTrackingData(0.0);

    auto expected = offset * rot;

    EXPECT_TRUE(pos.isApprox(control.getPosition()))
        << "Expected: " << pos.transpose()
        << " Actual: " << control.getPosition();
    EXPECT_TRUE(expected.isApprox(control.getOrientation()))
        << "Expected: " << expected.coeffs().transpose()
        << " Actual: " << control.getOrientation().coeffs().transpose();
}

TEST_F(TrackingDeviceControlTest, InvertTranslation)
{
    control.setInversionFlags(
            TrackingDeviceControl::InvertFlag::transX |
            TrackingDeviceControl::InvertFlag::transY |
            TrackingDeviceControl::InvertFlag::transZ);

    auto pos = Vec3d(1.0, 2.0, 3.0);
    auto rot = Quatd(4.0, 5.0, 6.0, 7.0).normalized();

    client->setPosition(pos);
    client->setOrientation(rot);

    control.updateTrackingData(0.0);

    EXPECT_TRUE((pos * -1.0).isApprox(control.getPosition()))
            << "Expected: " << (pos * -1).transpose() << " Actual: " << control.getPosition().transpose();
    EXPECT_TRUE(rot.isApprox(control.getOrientation()))
            << "Expected: " << rot.coeffs().transpose() << " Actual: " << control.getOrientation().coeffs().transpose();
}

TEST_F(TrackingDeviceControlTest, InvertOrientation)
{
    // For reference: iMSTK uses OpenGL which uses right handed system
    // +y
    // |  -z
    // | /
    // +-----> +x

    Vec3d                             dir[] = { Vec3d(1.0, 0.0, 0.0), Vec3d(0.0, 1.0, 0.0), Vec3d(0.0, 0.0, 1.0) };
    TrackingDeviceControl::InvertFlag invertFlags[] =
    {
        TrackingDeviceControl::InvertFlag::rotX,
        TrackingDeviceControl::InvertFlag::rotY,
        TrackingDeviceControl::InvertFlag::rotZ
        };

    // Inverts for rotations happen on a plane so invert X will flip Y & Z rotations.
    for (int i = 0; i < 3; i++)
    {
        // 45 deg rotation around one of the axes that is not this one (i+1)%3
        Quatd orientation = Quatd(Rotd(PI_2 * 0.5, dir[(i + 1) % 3]));

        client->setPosition(Vec3d(0.0, 0.0, 0.0));
        client->setOrientation(orientation);

        // Flip over dir
        control.setInversionFlags(invertFlags[i]);
        control.updateTrackingData(0.0);

        // Position should remain untouched
        EXPECT_TRUE(Vec3d(0.0, 0.0, 0.0).isApprox(control.getPosition()))
            << "Direction: " << dir[i].transpose() << std::endl
            << "Expected: " << Vec3d(0.0, 0.0, 0.0).transpose() << std::endl
            << "Actual: " << control.getPosition().transpose();

        // Expected
        Quatd expectedOrientation = Quatd(Rotd(-PI_2 * 0.5, dir[(i + 1) % 3]));
        Quatd actualOrientation   = control.getOrientation();
        EXPECT_TRUE(expectedOrientation.isApprox(actualOrientation))
            << "Direction: " << dir[i].transpose() << std::endl
            << "Expected: " << expectedOrientation << std::endl
            << "Actual: " << actualOrientation;
    }
}

TEST_F(TrackingDeviceControlTest, TranslationOffset)
{
    auto offset = Vec3d(4.0, 5.0, 6.0);
    control.setTranslationOffset(offset);

    auto pos = Vec3d(1.0, 2.0, 3.0);
    auto rot = Quatd(4.0, 5.0, 6.0, 7.0).normalized();

    client->setPosition(pos);
    client->setOrientation(rot);

    control.updateTrackingData(0.0);

    Vec3d expectedPos = pos + offset;
    Quatd expectedRot = rot;
    EXPECT_TRUE(expectedPos.isApprox(control.getPosition()))
        << "Expected: " << expectedPos.transpose()
        << " Actual: " << control.getPosition();
    EXPECT_TRUE(expectedRot.isApprox(control.getOrientation()))
        << "Expected: " << expectedRot.coeffs().transpose()
        << " Actual: " << control.getOrientation().coeffs().transpose();
}