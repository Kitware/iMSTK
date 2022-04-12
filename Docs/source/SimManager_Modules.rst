SimulationManager & Modules
===========================

SimulationManager: Usage
========================

iMSTK defines a set of Modules. Modules define something that can be initialized, updated, and uninitialized. These can be used standalone if needbe. The three major modules given by iMSTK are:

- VTKViewer
- SceneManager
- HapticManager
  
The intention here is that modules do not define their execution. They are "driven" separately because update schemes tend to vary across systems. Execution is done by a ModuleDriver. The ModuleDriver is an abstract class which starts and stops a collection of Modules. We provide one such concrete implementation called the SimulationManager. It's used throughout the examples. The basic setup is as such:

::

    imstkNew<VTKViewer> viewer;
    viewer->setActiveScene(scene);

    imstkNew<SceneManager> sceneManager;
    sceneManager->setActiveScene(scene);

    imstkNew<SimulationManager> driver;
    driver->addModule(viewer);
    driver->addModule(sceneManager);

    driver->start();


By default the simulation starts when start is called. You may, however, pause and resume any module, such as the SceneManager with:

::

    sceneManager->pause();
    sceneManager->resume();

Some simulators need a pause function, many don't ever pause and just run from the start of the app until closing, others implmenent their own concepts of state.

If you'd like to stop the entire SimulationManager you may submit a thread safe request to change it.

::

    // Request that it stop
    driver->requestStatus(ModuleDriverStopped);


Details of Physics In The Update Loop
===============

Consider a particle moving in one direction/dimensions on two separate machines/hardwares with differing processor speeds using the below function.

::

    void update(double dt)
    {
        position += velocity * dt
    }

If we have a constant velocity=5m/s. Consider a slow and fast computer.

- Fast Computer: Calls this function 6 times over 10 real life seconds.
- Slow Computer: Calls this function 3 times over 10 real life seconds.

The two computers have produced differing displacements of the particle despite the same amount of real time passing.

- Fast Computer: Particle moved 6x5=30 meters.
- Slow Computer: Particle moved 5x3=15 meters.

One naive but possible solution is to use our timestep (dt) here. We may use a dt=1 on the fast computer & a dt=2 on the slow computer. Assuming both computers processors were consistent we would get identical displacements. In actuality processors rarely run at deterministic speeds at this fine of a level. Intead, a common heuristic is to use the time the last update took as a prediction for the time the current update will take.

::

    Timer timer;
    timer.start();
    double dt = 0.0;
    while (running)
    {
        update(dt);
        dt = timer.getElapsedTime(); // Also resets timer
    }

This is your basic real time timestep. If the computer slows down we get larger dts. If it speeds up we get smaller dts. Even if we introduce more work, such as rendering after the update. We still account for the time that lost while rendering. This timestep may make some simulation engineers nervous though. This is for two reasons:
- We do not know what dt will be. Perhaps there is a lot of work done and dt gets quite large to catch up. This could easily cause a simulation to explode on a slow machine, and be fine on a fast one. Mins or maxs might be used. This sort of issues make portability and distribution very difficult.
- The results are always indeterministic. If the simulation is run twice on the same system. It could even produce very slightly differing answers.

The solution is something we like to call sequential substepping and is not exclusive to imstk. The idea is to "use a fixed timestep but vary the amount of update calls per render".

::

    Timer timer;
    timer.start();
    while(running)
    {
        timer.getElapsedTime();
        N = ... computed via time passed ...
        for (int i = 0; i < N; i++)
        {
            update(FIXED_DT);
        }
        render();
    }

This, for example, may allow us to take some 1000 simulation updates per render. The trick is that N varies with the machine. A faster computer may perform more updates per render. Where a slow computer may perform less. But both computers use the same unchanging timestep. Resulting in portable, non exploding, deterministic code.

To compute N we setup a time accumultor/time bank. We effectively add time as it passes to this accumulator. And dispense it when we do our updates. This keeps us up with real time.

::

    Timer timer;
    timer.start();
    double timeBank = 0.0;
    while(running)
    {
        timeBank += timer.getElapsedTime();
        N = static_cast<int>(timeBank / FIXED_DT); // Floored
        timeBank -= N * FIXED_DT;
        for (int i = 0; i < N; i++)
        {
            update(FIXED_DT);
        }
        render();
    }

You will notice that there will be a remainder. If 5s have passed and FIXED_DT=2s. We will have 1s remainder. This can cause a problem if your system is running consistently giving a remainder of 1s. Resulting in something like 2 updates, 3 updates, 2 updates, ... so forth. Sometimes this can be noticable, there are a few solutions, we have chosen to divide out the remainder over the N frames. Many implementations would favor determinism instead. Our resulting update loop looks like the following diagram.

.. image:: media/pipeline.png
    :width: 800
    :alt: Alternative text
    :align: center

To set the desired/fixed dt you may use:

::

    driver->setDesiredDt(0.003);

A timestep of 0.01 is pretty standard. 0.016 will give you 16ms updates for 60fps. If using haptics you may need a fast update rate. 1000hz (dt=0.001) is the gold standard. But you may get away with some looser haptics at ~500hz.

Lastly each module has a prefered/default execution mode as follows. It need not be specified by a user:

- Adaptive: Run N times per render. How N is computed and why is described below.
  
  - Such as the SceneManager.
- Sequential: Runs once per render.
  
  - Such as the VTKViewer.
- Parallel: Ran in a loop on another thread, unconstrained by the others.
  
  - Such as the HapticDeviceManager.

Though one can change it. Or even provide their own implementation of a ModuleDriver (their own SimulationManager).

Usage & Integration with External Systems
===============
Whilst iMSTK provides its own rendering backend it is often used for simulation alone. This will detail the different ways to use/integrate it.

Usage can be categorized into three groups:

Synchronously Run iMSTK:
--------------------

This would follow standard usage of iMSTK

::

    // Setup a viewer to render
    auto viewer = std::make_shared<VTKViewer>();
    viewer->setActiveScene(scene);

    // Setup a scene manager to advance the scene
    auto sceneManager = std::make_shared<SceneManager>();
    sceneManager->setActiveScene(scene);
    sceneManager->pause(); // Start simulation paused

    // Setup a SimulationManager to perform the updates of the given modules
    auto driver = std::make_shared<SimulationManager>();
    driver->addModule(viewer);
    driver->addModule(sceneManager);
    driver->setDesiredDt(0.001);

    driver->start(); // Starts synchronously/don't return until finished with simulation

You may also omit the VTKViewer entirely, this will avoid creation of a window and any related graphics. If you do this, the SimulationManager is equivalently just a very simple while loop like so and the SimulationManager serves little purpose:

::

    sceneManager->initialize();
    sceneManager->setDt(0.001);
    while (running)
    {
        sceneManager->update();
    }

Even further one is able to do:

::

    scene->initialize();
    while (running)
    {
        scene->advance(0.001);
    }

Asynchronously Run iMSTK:
--------------------

iMSTK does not support any internal mechanisms for async starting but the following is a possibility:

::

    std::thread thread = std::thread([&]() { driver->start(); }); // Starts synchronously/don't return until finished with simulation

    ... somewhere else in code ...
    driver->requestStatus(ModuleDriverStopped);
    thread.join();

Synchronously in someone elses event loop:
--------------------

As shown previously you may call updates/advance on your own with ease allowing you to run a simulation within your own, or another systems event loop.

Note on Real Time Systems
===============

Although iMSTK uses VTK which by default runs event based rendering (Meaning events invoke render calls). iMSTK does not use VTKs event based rendering. Event based rendering tends to be suitable for many UI applications that want to avoid rendering when nothing has changed on screen, saving battery, freeing up the processor for other tasks, allowing better mulitasking for other UIs on a machine. Games do not often use event based rendering. This mostly comes down to scheduling techniques on real time systems (very related to how your operating system is a real time system, constantly balancing tasks the OS has no idea how long will take). **Most** event based systems aren't the optimal scheduling technique for games. Games are often composed of a set of fixed subsystem/modules that do not change and for which we can quantify how much work is to be done. Physics, rendering, input, animation all are updated in a given pipeline that is cyclic executive, touched only once in sequence repetively, providing absolute determinism to when the next one will occur with no scheduling overhead (managing priorities, alternation schemes, no pre-emption other than OS's, etc). All that being said, nothing stops one from putting iMSTK into any event loop (VTKs included).