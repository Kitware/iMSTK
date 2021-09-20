using System;
using imstk;

public class PbdCloth
{
    private static int SCENE_ID = 1;

    public static void Main(string[] args)
    {

        // Setup logger (write to file and stdout)
        Logger.startLogger();

        double particleRadius = 0.1;

        // Override particle radius for scene3 because particles in this scene were pre-generated using particle radius 0.08
        if (SCENE_ID == 3)
        {
            particleRadius = 0.08;
        }

        Scene scene = new Scene("SPH Fluid");

        SPHObject fluidObj = generateFluid(particleRadius);
        CollidingObject[] solids = generateSolids(scene);
        scene.addSceneObject(fluidObj);
        for (int i = 0; i < solids.Length; i++)
        {
            scene.addSceneObject(solids[i]);
        }

        // Collision between fluid and solid objects
        CollisionGraph collisionGraph = scene.getCollisionGraph();

        collisionGraph.addInteraction(new SphObjectCollision(fluidObj, solids[0]));
        collisionGraph.addInteraction(new SphObjectCollision(fluidObj, solids[1]));
        collisionGraph.addInteraction(new SphObjectCollision(fluidObj, solids[2]));


        // configure camera
        scene.getActiveCamera().setPosition(-0.475, 8.116, -6.728);

        // configure light (white)
        DirectionalLight whiteLight = new DirectionalLight();
        whiteLight.setFocalPoint(new Vec3d(5.0, -8.0, -5.0));
        whiteLight.setIntensity(1.5);
        scene.addLight("whiteLight", whiteLight);

        // Run the simulation
        {
            // Setup a viewer to render
            VTKViewer viewer = new VTKViewer("Viewer");
            viewer.setActiveScene(scene);
            viewer.setWindowTitle("SPH Fluid");
            viewer.setSize(1920, 1080);
            VTKTextStatusManager statusManager = viewer.getTextStatusManager();
            statusManager.setStatusFontSize(VTKTextStatusManager.StatusType.Custom, 30);
            statusManager.setStatusFontColor(VTKTextStatusManager.StatusType.Custom, Color.Red);

            // Action<KeyEvent> receiverAction = (KeyEvent e) => {
            //     statusManager.setCustomStatus("Number of particles: " +
            //                                    std.to_string(fluidObj.getSPHModel().getCurrentState().getNumParticles()) +
            //                                    "\nNumber of solids: " + std.to_string(solids.size()));
            //
            // };

            // CSReceiverFunc eventFunc = new CSReceiverFunc(receiverAction);
            // Utils.connectEvent(viewer, Utils.VTKViewer_getPostUpdate_cb, eventFunc);


            // Setup a scene manager to advance the scene
            SceneManager sceneManager = new SceneManager("Scene Manager");
            sceneManager.setActiveScene(scene);
            sceneManager.pause();

            SimulationManager driver = new SimulationManager();
            driver.addModule(viewer);
            driver.addModule(sceneManager);

            // Add mouse and keyboard controls to the viewer
            {
                MouseSceneControl mouseControl = new MouseSceneControl(viewer.getMouseDevice());
                mouseControl.setSceneManager(sceneManager);
                viewer.addControl(mouseControl);

                KeyboardSceneControl keyControl = new KeyboardSceneControl(viewer.getKeyboardDevice());
                keyControl.setSceneManager(new SceneManagerWeakPtr(sceneManager));
                keyControl.setModuleDriver(new ModuleDriverWeakPtr(driver));
                viewer.addControl(keyControl);
            }

            driver.start();
        }
    }

    public static SPHObject generateFluid(double particleRadius)
    {
        VecDataArray3d particles = new VecDataArray3d();
        switch (SCENE_ID)
        {
        case 1:
            particles = generateSphereShapeFluid(particleRadius);
            break;
        case 2:
            particles = generateBoxShapeFluid(particleRadius);
            break;
        case 3:
            // particles = generateBunnyShapeFluid(particleRadius);
            return null;
            break;
        default:
            return null;
        }


        // Create a geometry object
        PointSet geometry = new PointSet();
        geometry.initialize(particles);

        // Create a fluids object
        SPHObject fluidObj = new SPHObject("Sphere");

        // Create a visual model
        VisualModel visualModel = new VisualModel(geometry);
        RenderMaterial material = new RenderMaterial();
        material.setDisplayMode(RenderMaterial.DisplayMode.Fluid);
        //material.setDisplayMode(RenderMaterial.DisplayMode.Points);
        if (material.getDisplayMode() == RenderMaterial.DisplayMode.Fluid)
        {
            material.setPointSize(0.1f);
        }
        else
        {
            material.setPointSize(20.0f);
            material.setRenderPointsAsSpheres(true);
            material.setColor(Color.Orange);
        }
        visualModel.setRenderMaterial(material);

        // Create a physics model
        SPHModel sphModel = new SPHModel();
        sphModel.setModelGeometry(geometry);

        // Configure model
        SPHModelConfig sphParams = new SPHModelConfig(particleRadius);
        sphParams.m_bNormalizeDensity = true;
        if (SCENE_ID == 2)   // highly viscous fluid
        {
            sphParams.m_kernelOverParticleRadiusRatio = 6.0;
            sphParams.m_surfaceTensionStiffness = 5.0;
        }

        if (SCENE_ID == 3)   // bunny-shaped fluid
        {
            sphParams.m_frictionBoundary = 0.3;
        }

        sphModel.configure(sphParams);
        sphModel.setTimeStepSizeType(TimeSteppingType.RealTime);

        // Add the component models
        fluidObj.addVisualModel(visualModel);
        fluidObj.setCollidingGeometry(geometry);
        fluidObj.setDynamicalModel(sphModel);
        fluidObj.setPhysicsGeometry(geometry);

        return fluidObj;
    }

    ///
    /// \brief Generate a sphere-shape fluid object
    ///
    public static VecDataArray3d generateSphereShapeFluid(double particleRadius)
    {
        double sphereRadius = 2.0;
        Vec3d  sphereCenter = new Vec3d(0, 1, 0);
        double  sphereRadiusSqr = sphereRadius * sphereRadius;
        double  spacing = 2.0 * particleRadius;
        int     N = (int)(2.0 * sphereRadius / spacing);              // Maximum number of particles in each dimension
        // Vec3d lcorner = sphereCenter - new Vec3d(sphereRadius, sphereRadius, sphereRadius); // Cannot use auto here, due to Eigen bug
        Vec3d lcorner = new Vec3d(sphereCenter[0] - sphereRadius, sphereCenter[1] - sphereRadius, sphereCenter[2] - sphereRadius); // Cannot use auto here, due to Eigen bug

        VecDataArray3d particles = new VecDataArray3d();
        particles.reserve(N * N * N);

        for (int i = 0; i < N; ++i)
        {
            for (int j = 0; j < N; ++j)
            {
                for (int k = 0; k < N; ++k)
                {
                    Vec3d ppos = lcorner + new Vec3d(spacing * (double)(i), spacing * (double)(j), spacing * (double)(k));
                    Vec3d cx = ppos - sphereCenter;
                    double nrm = cx[0] * cx[0] + cx[1] * cx[1] + cx[2] * cx[2];
                    if (nrm < sphereRadiusSqr)
                    {
                        particles.push_back(ppos);
                    }
                }
            }
        }

        return particles;
    }

    ///
    /// \brief Generate a box-shape fluid object
    ///
    public static VecDataArray3d generateBoxShapeFluid(double particleRadius)
    {
        double boxWidth = 4.0;
        Vec3d  boxLowerCorner = new Vec3d(-2, -3, -2);

        double spacing = 2.0 * particleRadius;
        int N = (int)(boxWidth / spacing);

        VecDataArray3d particles = new VecDataArray3d();
        particles.reserve(N * N * N);

        for (int i = 0; i < N; ++i)
        {
            for (int j = 0; j < N; ++j)
            {
                for (int k = 0; k < N; ++k)
                {
                    Vec3d ppos = boxLowerCorner + new Vec3d(spacing * i, spacing * j, spacing * k);
                    particles.push_back(ppos);
                }
            }
        }

        return particles;
    }

    public static CollidingObject[] generateSolids(Scene scene)
    {
        switch (SCENE_ID)
        {
        case 1:
            return generateSolidsScene1();
        case 2:
            return null; // To avoid warning
            // return generateSolidsScene2();
        case 3:
            return null; // To avoid warning
            // return generateSolidsScene3();
        case 4:
            return null; // To avoid warning
            // return generateSolidsScene4(scene);
        default:
            return null; // To avoid warning
        }
    }

    ///
    /// \brief Generate two planes and a solid sphere
    ///
    public static CollidingObject[] generateSolidsScene1()
    {
        CollidingObject[] solids = new CollidingObject[3];

        {
            Plane geometry = new Plane();
            geometry.setWidth(40.0);
            geometry.setPosition(0.0, -6.0, 0.0);
            geometry.setNormal(new Vec3d(0.0, 1.0, -0.5));

            VisualModel visualModel = new VisualModel(geometry);
            RenderMaterial material = new RenderMaterial();
            material.setColor(Color.DarkGray);
            visualModel.setRenderMaterial(material);

            CollidingObject obj = new CollidingObject("Floor");
            obj.addVisualModel(visualModel);
            obj.setCollidingGeometry(geometry);
            solids[0] = obj;
        }
        {
            Plane geometry = new Plane();
            geometry.setWidth(40.0);
            geometry.setPosition(0.0, -6.0, 0.0);
            geometry.setNormal(new Vec3d(0.0, 1.0, 1.0));

            VisualModel visualModel = new VisualModel(geometry);
            RenderMaterial material = new RenderMaterial();
            material.setColor(Color.LightGray);
            visualModel.setRenderMaterial(material);

            CollidingObject obj = new CollidingObject("Back Plane");
            obj.addVisualModel(visualModel);
            obj.setCollidingGeometry(geometry);
            solids[1] = obj;
        }
        {
            Sphere geometry = new Sphere();
            geometry.setRadius(2.0);
            geometry.setPosition(0.0, -6.0, 0.0);

            VisualModel visualModel = new VisualModel(geometry);
            RenderMaterial material = new RenderMaterial();
            material.setColor(Color.Red);
            visualModel.setRenderMaterial(material);

            CollidingObject obj = new CollidingObject("Sphere on Floor");
            obj.addVisualModel(visualModel);
            obj.setCollidingGeometry(geometry);
            solids[2] = obj;
        }

        return solids;
    }
}
