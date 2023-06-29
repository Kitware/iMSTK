/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkOpenHapticDeviceManager.h"
#include "imstkOpenHapticDeviceClient.h"
#include "imstkLogger.h"

#include <HD/hd.h>

#include <iostream>
#include <iomanip>

namespace
{
struct HDstate
{
    // \todo pos are redundant?
    HDdouble pos[3];
    HDdouble vel[3];
    HDdouble angularVel[3];
    HDdouble transform[16];
    HDint buttons;
};
} // namespace

namespace imstk
{
class OpenHapticDeviceManager::OpenHapticDeviceManagerImpl
{
public:

    /// Check for OpenHaptics HDAPI errors, display them, and signal fatal errors.
    /// \param message An additional descriptive message.
    /// \return true if there was a fatal error; false if everything is OK.
    static bool isFatalError(const char* message)
    {
        HDErrorInfo error = hdGetError();
        if (error.errorCode == HD_SUCCESS)
        {
            return false;
        }

        // The HD API maintains an error stack, so in theory there could be more than one error pending.
        // We do head recursion to get them all in the correct order, and hope we don't overrun the stack...
        bool anotherFatalError = isFatalError(message);

        bool isFatal = ((error.errorCode != HD_WARM_MOTORS)
                        && (error.errorCode != HD_EXCEEDED_MAX_FORCE)
                        && (error.errorCode != HD_EXCEEDED_MAX_FORCE_IMPULSE)
                        && (error.errorCode != HD_EXCEEDED_MAX_VELOCITY)
                        && (error.errorCode != HD_FORCE_ERROR));

        LOG(WARNING) << "Phantom: " << message <<
            std::endl << "  Error text: '" << hdGetErrorString(error.errorCode) << "'" << std::endl <<
            "  Error code: 0x" << std::hex << std::setw(4) << std::setfill('0') << error.errorCode <<
            " (internal: " << std::dec << error.internalErrorCode << ")" << std::endl;

        return (isFatal || anotherFatalError);
    }

    static HDCallbackCode HDCALLBACK hapticCallback(void* pData)
    {
        auto    impl = static_cast<OpenHapticDeviceManagerImpl*>(pData);
        HDstate state;

        for (int num = 0; num < impl->m_deviceClients.size(); ++num)
        {
            HHD                     handle = impl->m_handles[num];
            OpenHapticDeviceClient* client = impl->m_deviceClients[num].get();

            if (handle == HD_BAD_HANDLE || handle == HD_INVALID_HANDLE)
            {
                continue;
            }
            Vec3d force = client->getForce();

#ifdef IMSTK_OPENHAPTICS_DEBUG
            if (force.hasNaN())
            {
                force = Vec3d::Zero();
                LOG(WARNING) << "Force has NANs";
            }
#endif
            hdBeginFrame(handle);

            //hdMakeCurrentDevice(handle);
            hdSetDoublev(HD_CURRENT_FORCE, force.data());
            hdGetDoublev(HD_CURRENT_POSITION, state.pos);
            hdGetDoublev(HD_CURRENT_VELOCITY, state.vel);
            hdGetDoublev(HD_CURRENT_ANGULAR_VELOCITY, state.angularVel);
            hdGetDoublev(HD_CURRENT_TRANSFORM, state.transform);
            hdGetIntegerv(HD_CURRENT_BUTTONS, &state.buttons);

            hdEndFrame(handle);

            CHECK(!isFatalError("Error in device update"));

            // Update client data from state data
            const Quatd orientation = Quatd((Eigen::Affine3d(Eigen::Matrix4d(state.transform))).rotation());
            client->m_transformLock.lock();
            // OpenHaptics is in mm, change to meters
            client->m_position << state.pos[0] * 0.001, state.pos[1] * 0.001, state.pos[2] * 0.001;
            client->m_velocity << state.vel[0] * 0.001, state.vel[1] * 0.001, state.vel[2] * 0.001;
            client->m_angularVelocity << state.angularVel[0], state.angularVel[1], state.angularVel[2];
            client->m_orientation = orientation;
            client->m_transformLock.unlock();

            client->m_dataLock.lock();
            for (int i = 0; i < 4; i++)
            {
                // If button down and not previously down
                if ((state.buttons & (1 << i)) && !client->m_buttons[i])
                {
                    client->m_buttons[i] = true;
                    client->m_events.push_back({ i, BUTTON_PRESSED });
                }
                // If button not down, and previously down
                else if (!(state.buttons & (1 << i)) && client->m_buttons[i])
                {
                    client->m_buttons[i] = false;
                    client->m_events.push_back({ i, BUTTON_RELEASED });
                }
            }
            client->m_dataLock.unlock();
        }

        return HD_CALLBACK_CONTINUE;
    }

    std::shared_ptr<imstk::OpenHapticDeviceClient> makeDeviceClient(std::string name)
    {
        auto client = std::make_shared<imstk::OpenHapticDeviceClient>(name);
        m_deviceClients.push_back(client);
        return client;
    }

    bool init()
    {
        for (const auto& client : m_deviceClients)
        {
            client->initialize();

            //flush error stack
            HDErrorInfo errorFlush;
            while (HD_DEVICE_ERROR(errorFlush = hdGetError())) {}

            HHD handle;

            auto name = client->getDeviceName();
            // Initialize the device
            if (name == "")
            {
                handle = hdInitDevice(HD_DEFAULT_DEVICE);
            }
            else
            {
                handle = hdInitDevice(name.c_str());
            }

            CHECK(!isFatalError("Failed to initialize device"));

            m_handles.push_back(handle);

            hdMakeCurrentDevice(handle);

            // If initialized as default, set the name
            if (name == "")
            {
                // Worth noting that in this case the name will not match the actual device name and is
                // now only useful for scene level identification, OpenHaptics provides no mechanisms
                // for querying device names

                HDstring str = hdGetString(HD_DEVICE_SERIAL_NUMBER);
                client->setDeviceName("Device_" + std::string(str));
            }

            // Enable forces
            hdEnable(HD_FORCE_OUTPUT);
            hdEnable(HD_FORCE_RAMPING);

            CHECK(!isFatalError("Failed to enable forces"));

            LOG(INFO) << "\"" << client->getDeviceName() << "\" successfully initialized.";
        }

        // Start the scheduler
        m_schedulerHandle = hdScheduleAsynchronous(hapticCallback, this, HD_MAX_SCHEDULER_PRIORITY); // Call sometime later
        hdStartScheduler();
        CHECK(!isFatalError("Failed to schedule callback"));
        return true;
    }

    void update()
    {
        for (const auto& client : m_deviceClients)
        {
            client->update();
        }
    }

    void uninit()
    {
        hdStopScheduler();
        hdUnschedule(m_schedulerHandle);
        for (const auto handle : m_handles)
        {
            hdDisableDevice(handle);
            CHECK(!isFatalError("Failed to Disable device"));
        }
    }

private:
    HDSchedulerHandle m_schedulerHandle = 0;
    std::vector<std::shared_ptr<imstk::OpenHapticDeviceClient>> m_deviceClients; ///< list of all the device clients
    std::vector<HHD> m_handles;                                                  ///< a device handle for each client
};

OpenHapticDeviceManager::OpenHapticDeviceManager() :
    m_impl(new OpenHapticDeviceManagerImpl)
{
    // Default a 1ms sleep to avoid over consumption of the CPU
    setSleepDelay(1.0);
    setMuteUpdateEvents(true);
}

std::shared_ptr<DeviceClient>
OpenHapticDeviceManager::makeDeviceClient(std::string name)
{
    if (getInit())
    {
        LOG(WARNING) << "Can't add device client after initialization.";
        return nullptr;
    }
    return m_impl->makeDeviceClient(name);
}

bool
OpenHapticDeviceManager::initModule()
{
    if (getInit())
    {
        LOG(WARNING) << "OpenHapticDeviceManager already initialized. Reinitialization not implemented.";
        return false;
    }
    return m_impl->init();
}

void
OpenHapticDeviceManager::updateModule()
{
    m_impl->update();
}

void
OpenHapticDeviceManager::uninitModule()
{
    m_impl->uninit();
}
} // namespace imstk