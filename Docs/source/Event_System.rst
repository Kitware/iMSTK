Event System
============

Event systems have various pros and cons. iMSTK recently added one but its usage is still minimal. As a user you will mostly see/use it for devices.

With our events, an object that has base class type EventObject may send and receive events. Additionally function pointers may be used for "direct" receivers.

The "connect" function is used to add a receiver/observer to a sender. There are two ways to observe a sender. Queued or direct.

- Direct: When the event is emitted, all direct observers are called immediately.
    - It happens in sync. Sequentially. Does not return from the event emit until receivers are all called.
    - It happens on the same thread it was emitted from.
- Queued: When the event is emitted, all queued observers receive the event in a message queue.
    - It happens async. Handled sometime later. The observer must implement the pop and handle of the event.
    - It can switch threads or even machines.


Consider a KeyboardDeviceClient. It may emit a KeyEvent.

.. code:: c++

    std::shared_ptr<KeyboardDeviceClient> myKeyboardDevice = ...
    connect<KeyEvent>(myKeyboardDevice, &KeyboardDeviceClient::keyPress, [&](KeyEvent* e)
    {
        printf("action %d occured on key %d\n", e->m_keyPressType, e->m_key);
    });

This function is called immediately and on the same thread when the event happens.

Additionally you may direct connect to the function of an object.

::

    std::shared_ptr<KeyboardDeviceClient> myKeyboardDevice = ...
    connect<KeyEvent>(myKeyboardDevice, 
                      &KeyboardDeviceClient::keyPress, 
                      myCustomObject, 
                      &MyCustomObject::myCustomFunction);

Alternatively we could queue it to another object like so:

::

    std::shared_ptr<KeyboardDeviceClient> myKeyboardDevice = ...
    queueConnect<KeyEvent>(myKeyboardDevice, &KeyboardDeviceClient::keyPress, sceneManager, [&](KeyEvent* e)
    {
        printf("action %d occured on key %d\n", e->m_keyPressType, e->m_key);
    });

This function is then called whenever sceneManager handles the event. Which would be before it does a scene advance.

Queued Event Example 1
-----------------------

Here we setup a device that will emit a button event and stapler is made to listen to it.

::

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

    // Setup a haptic device
    imstkNew<HapticDeviceManager> hapticManager;
    std::shared_ptr<HapticDeviceClient> hapticClient = hapticManager->makeDeviceClient();
    
    // Setup our stapler
        imstkNew<StaplerObject> stapler;
        scene->addSceneObject(stapler);

    // Connect stapler
    queueConnect<ButtonEvent>(hapticClient, 
                              &HapticDeviceClient::buttonStateChanged, 
                              stapler, 
                              &StaplerObject::staple);

    ... Setup SimulationManager and start ...
    }


Queued Event Example 2
-----------------------

Here we emit a custom event for a tool. I put when the object is touching, but it could really be anything. Keep in mind we can either directly handle the vent, or queue it. Queuing is generally safer but not as fast.

::

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

        imstkNew<ToolObject> myToolObject;
        scene->addSceneObject(myToolObject);

        // We could queue it to anything, even another SceneObject, here we queue it
        // directly to the sceneManager as we know it will handle its events every update
        queueConnect<Event>(myToolObject, &ToolObject::isTouching, sceneManager, [&](Event* e)
        {
            // Do stuff for when it touches
        });

        ... Setup SimulationManager and start ...
    }