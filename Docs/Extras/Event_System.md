# Event System

Event systems have various pros and cons. iMSTK has one that it minimally uses internally. With iMSTK events, an object that has base class type EventObject may send and receive events.

The "connect" function is used to add a receiver/observer to a sender. There are two ways to observe a sender. Queued or direct.

- Direct: When the event is emitted, all direct observers are called immediately.
    - It happens in sync. Sequentially. Does not return from the event emit until receivers are all called.
    - It happens on the same thread it was emitted from.
- Queued: When the event is emitted, all queued observers receive the event in a message queue.
    - It happens async. Handled sometime later. The observer must implement the pop and handle of the event.
    - It can switch threads or even machines.

Consider a KeyboardDeviceClient. It may emit a KeyEvent.

```cpp
std::shared_ptr<KeyboardDeviceClient> myKeyboardDevice = viewer->getKeyboardDevice();
connect<KeyEvent>(myKeyboardDevice, &KeyboardDeviceClient::keyPress,
[&](KeyEvent* e)
{
    printf("action %d occured on key %d\n", e->m_keyPressType, e->m_key);
});
```

This function is called immediately and on the same thread when the event happens.

Additionally you may direct connect to the function of an object instead of using a C++ lambda.

```cpp
std::shared_ptr<KeyboardDeviceClient> myKeyboardDevice = viewer->getKeyboardDevice();
connect<KeyEvent>(myKeyboardDevice, 
    &KeyboardDeviceClient::keyPress, 
    myCustomObject, 
    &MyCustomObject::myCustomFunction);
```

Alternatively queue it to another object like so:

```cpp
std::shared_ptr<KeyboardDeviceClient> myKeyboardDevice = viewer->getKeyboardDevice();
queueConnect<KeyEvent>(myKeyboardDevice, &KeyboardDeviceClient::keyPress, sceneManager,
[&](KeyEvent* e)
{
    printf("action %d occured on key %d\n", e->m_keyPressType, e->m_key);
});
```

In this case, the function is not run until sceneManager processes its event queue. SceneManager can process its event queue when it wants too.

## Queued Event Example 1

Here a device is setup that will emit a button event and stapler is made to listen to it.

```cpp
class StaplerObject : public SceneObject
{
public:
    void staple(ButtonEvent* e)
    {
        // perform staple;
    }

    void update() override
    {
        doAllEvents();
    }
};

void main()
{
    ... Scene Setup ...

    // Setup default haptics manager
    std::shared_ptr<DeviceManager> hapticManager = DeviceManagerFactory::makeDeviceManager();
    std::shared_ptr<DeviceClient>  deviceClient  = hapticManager->makeDeviceClient();

    // Setup our stapler
    auto stapler = std::make_shared<StaplerObject>();
    scene->addSceneObject(stapler);

    // Connect hapticClient buttonStateChanged event to stapler's staple slot
    queueConnect<ButtonEvent>(hapticClient, &HapticDeviceClient::buttonStateChanged, 
        stapler, &StaplerObject::staple);

    ... Setup SimulationManager and start ...
}
```


## Queued Event Example 2

Here a custom event is emitted for a tool when the object is touching. Either directly handle the event, or queue it. Queuing is generally safer but not as fast.

```cpp
class ToolObject : public SceneObject
{
public:
    SIGNAL(ToolObject, isTouching);

public:
    void update() override
    {
        if (geometry is touching)
            this->postEvent(vent(ToolObject::isTouching()));
    }
};

void main()
{
    ... Scene Setup ...

    auto myToolObject = std::make_shared<ToolObject>();
    scene->addSceneObject(myToolObject);

    // We could queue it to anything, even another SceneObject, here we queue it
    // directly to the sceneManager as we know it will handle its events every update
    queueConnect<Event>(myToolObject, &ToolObject::isTouching, sceneManager,
        [&](Event* e)
        {
            // Do stuff for when it touches
        });

    ... Setup SimulationManager and start ...
}
```