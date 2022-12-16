using System;
using Imstk;

public class PbdCutting
{
    private static double width = 50.0;
    private static double height = 50.0;
    private static int nRows = 12;
    private static int nCols = 12;

    public static void Main(string[] args)
    {
        // Write log to stdout and file
        Logger.startLogger();

        // Setup a scene
        Scene scene = new Scene("PBDCutting");
        SurfaceMesh cutGeom = Utils.toTriangleGrid(new Vec3d(0.0, 0.0, 0.0), new Vec2d(40, 40), new Vec2i(2, 2));
        cutGeom.setTranslation(new Vec3d(-10.0, -20.0, 0.0));
        cutGeom.updatePostTransformData();
        SceneObject cutObj = new SceneObject("CuttingObject");
        cutObj.setVisualGeometry(cutGeom);
        cutObj.addComponentCollider().setGeometry(cutGeom);
        cutObj.getVisualModel(0).getRenderMaterial().setDisplayMode(RenderMaterial.DisplayMode.WireframeSurface);
        scene.addSceneObject(cutObj);

        PbdObject clothObj = makeClothObj("Cloth", width, height, nRows, nCols);
        scene.addSceneObject(clothObj);

        // Add interaction pair for pbd cutting
        PbdObjectCutting cuttingPair = new PbdObjectCutting(clothObj, cutObj);

        DeviceManager hapticManager = DeviceManagerFactory.makeDeviceManager();
        DeviceClient client = hapticManager.makeDeviceClient();

        SceneObjectController controller = new SceneObjectController();
        controller.setControlledObject(cutObj);
        controller.setDevice(client);
        scene.addControl(controller);

        // Adjust camera
        scene.getActiveCamera().setPosition(100.0, 100.0, 100.0);
        scene.getActiveCamera().setFocalPoint(0.0, -50.0, 0.0);

        // Light
        DirectionalLight light = new DirectionalLight();
        light.setFocalPoint(new Vec3d(5.0, -8.0, -5.0));
        light.setIntensity(1.0);
        scene.addLight("light", light);

        // Run the simulation
        {
            // Setup a viewer to render
            VTKViewer viewer = new VTKViewer("Viewer");
            viewer.setActiveScene(scene);

            // Setup a scene manager to advance the scene
            SceneManager sceneManager = new SceneManager("Scene Manager");
            sceneManager.setActiveScene(scene);
            sceneManager.setExecutionType(Module.ExecutionType.ADAPTIVE);
            sceneManager.pause(); // Start simulation paused

            SimulationManager driver = new SimulationManager();
            driver.addModule(hapticManager);
            driver.addModule(viewer);
            driver.addModule(sceneManager);
            driver.setDesiredDt(0.001);

            // Add mouse and keyboard controls to the viewer
            {
                MouseSceneControl mouseControl = new MouseSceneControl();
                mouseControl.setDevice(viewer.getMouseDevice());
                mouseControl.setSceneManager(sceneManager);
                scene.addControl(mouseControl);

                KeyboardSceneControl keyControl = new KeyboardSceneControl();
                keyControl.setDevice(viewer.getKeyboardDevice());
                keyControl.setSceneManager(new SceneManagerWeakPtr(sceneManager));
                keyControl.setModuleDriver(new ModuleDriverWeakPtr(driver));
                scene.addControl(keyControl);
            }

            Utils.connectKeyEvent(viewer.getKeyboardDevice(), Utils.KeyboardDeviceClient_getKeyPress_cb,
                (KeyEvent e) =>
                {
                    const int KEY_PRESS = 1;
                    // Set new textures
                    if (e.m_key == 'i' && e.m_keyPressType == KEY_PRESS)
                    {
                        cuttingPair.apply();
                    }
                });

            driver.start();
        }
    }

    public static PbdObject makeClothObj(string name, double width, double height, int rowCount, int colCount)
    {
        PbdObject clothObj = new PbdObject(name);

        // Setup the Geometry
        SurfaceMesh clothMesh = Utils.toTriangleGrid(new Vec3d(0.0, 0.0, 0.0),
            new Vec2d(width, height), new Vec2i(nRows, nCols));

        // Setup the Parameters
        PbdModelConfig pbdParams = new PbdModelConfig();
        pbdParams.enableConstraint(PbdModelConfig.ConstraintGenType.Distance, 1.0e3);
        pbdParams.enableConstraint(PbdModelConfig.ConstraintGenType.Dihedral, 1.0e3);
        pbdParams.m_gravity    = new Vec3d(0.0, -9.8, 0.0);
        pbdParams.m_dt  = 0.005;
        pbdParams.m_iterations = 5;

        // Setup the Model
        PbdModel pbdModel = new PbdModel();
        pbdModel.setModelGeometry(clothMesh);
        pbdModel.configure(pbdParams);

        // Setup the VisualModel
        RenderMaterial material = new RenderMaterial();
        material.setBackFaceCulling(false);
        material.setDisplayMode(RenderMaterial.DisplayMode.WireframeSurface);

        VisualModel visualModel = new VisualModel();
        visualModel.setGeometry(clothMesh);
        visualModel.setRenderMaterial(material);

        // Setup the Object
        clothObj.addVisualModel(visualModel);
        clothObj.setPhysicsGeometry(clothMesh);
        clothObj.addComponentCollider().setGeometry(clothMesh);
        clothObj.setDynamicalModel(pbdModel);

        clothObj.getPbdBody().fixedNodeIds = new VectorInt(2);
        clothObj.getPbdBody().fixedNodeIds.Add(0);
        clothObj.getPbdBody().fixedNodeIds.Add(colCount - 1);
        clothObj.getPbdBody().uniformMassValue = width * height / (rowCount * colCount);

        return clothObj;
    }
}