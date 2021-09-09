/*=========================================================================

   Library: iMSTK

   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
   & Imaging in Medicine, Rensselaer Polytechnic Institute.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

=========================================================================*/

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
    EXPECT_TRUE(rot.isApprox(control.getRotation()));
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

    EXPECT_TRUE(pos.isApprox(control.getPosition())) << "Expected: " << pos.transpose() << " Actual: " << control.getPosition();
    EXPECT_TRUE(expected.isApprox(control.getRotation())) << "Expected: " << expected.coeffs().transpose() << " Actual: " << control.getRotation().coeffs().transpose();
}

TEST_F(TrackingDeviceControlTest, InvertTranslation)
{
    unsigned char flags = static_cast<unsigned char>(TrackingDeviceControl::InvertFlag::transX) |
                          static_cast<unsigned char>(TrackingDeviceControl::InvertFlag::transY) |
                          static_cast<unsigned char>(TrackingDeviceControl::InvertFlag::transZ);

    control.setInversionFlags(flags);

    auto pos = Vec3d(1.0, 2.0, 3.0);
    auto rot = Quatd(4.0, 5.0, 6.0, 7.0).normalized();

    client->setPosition(pos);
    client->setOrientation(rot);

    control.updateTrackingData(0.0);

    EXPECT_TRUE((pos * -1.0).isApprox(control.getPosition())) << "Expected: " << (pos * -1).transpose() << " Actual: " << control.getPosition().transpose();
    EXPECT_TRUE(rot.isApprox(control.getRotation())) << "Expected: " << rot.coeffs().transpose() << " Actual: " << control.getRotation().coeffs().transpose();
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
        << "Expected: " << expectedPos.transpose() << " Actual: " << control.getPosition();
    EXPECT_TRUE(expectedRot.isApprox(control.getRotation()))
        << "Expected: " << expectedRot.coeffs().transpose() << " Actual: " << control.getRotation().coeffs().transpose();
}
