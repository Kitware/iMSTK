Controllers
===========

While devices give values and events they implement no function. This is the controller's job.

In the devices section we saw that we can directly connect to and query the devices. While its great for prototyping, generally doing so in the :code:`main()` is bad. Instead we should subclass. This is a part of iMSTK that is most commonly subclassed, making this section especially useful.

We provide a couple base classes for controls.

- MouseControl: For mouse controls
- KeyboardControl: For key controls
- TrackingDeviceControl: For tracking (positional/orientation) controls (OpenVR or Haptics devices)

Lets take a look at subclasses of each

::

    class MyCustomMouseControl : public MouseControl
    {
    public:
        MyCustomMouseControl() { }
        MyCustomMouseControl(std::shared_ptr<KeyboardDeviceClient> keyDeviceClient) : MouseControl(keyDeviceClient) { }
        ~MyCustomMouseControl() override = default;

    public:
        void OnButtonPress(const int key) override { }
        void OnButtonRelease(const int key) override { }
        void OnScroll(const double dx) override { }
        void OnMouseMove(const Vec3d pos) { }
    };

    void main()
    {
        ...

        // In main we could set it up like so:
        imstkNew<MyCustomMouseControl> control(viewer->getMouseDevice());
        
        // We can add the controller to the viewer to be processed before/after render
        viewer->addControl(control);
        
        // Alternatively we can add to the sceneManager to be procesed before/after scene is processed
        sceneManager->addControl(control);

        ...
    }


Here we can override each of the functions of a MouseControl and implement things we want. Similarly, we can do the same for KeyboardControl.

TrackingDeviceControls are a bit different though. They must be added to the scene. See the :code`LaparoscopicToolController` class which is a subclass of :code`TrackingDeviceControl`.

::

    imstkNew<LaparoscopicToolController> controller(objShaft, 
                                                    objUpperJaw, 
                                                    objLowerJaw, 
                                                    hapticDeviceClient);
    scene->addController(controller);


We also provide two common controls.

- MouseSceneControl
- KeyboardSceneControl

These are used in most examples and define the default controls. The :code:`MouseSceneControl` gives camera rotation controls and :code:`KeyboardSceneControl` gives stop, start, pause, and some other key controls.