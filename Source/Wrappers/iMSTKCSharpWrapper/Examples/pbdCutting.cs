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
        SurfaceMesh cutGeom = makeClothGeometry(40, 40, 2, 2);
        cutGeom.setTranslation(new Vec3d(-10.0, -20.0, 0.0));
        cutGeom.updatePostTransformData();
        CollidingObject cutObj = new CollidingObject("CuttingObject");
        cutObj.setVisualGeometry(cutGeom);
        cutObj.setCollidingGeometry(cutGeom);
        cutObj.getVisualModel(0).getRenderMaterial().setDisplayMode(RenderMaterial.DisplayMode.WireframeSurface);
        scene.addSceneObject(cutObj);

        PbdObject clothObj = makeClothObj("Cloth", width, height, nRows, nCols);
        scene.addSceneObject(clothObj);

        // Add interaction pair for pbd cutting
        PbdObjectCutting cuttingPair = new PbdObjectCutting(clothObj, cutObj);

        // Device Sever
        HapticDeviceManager server = new HapticDeviceManager();
        HapticDeviceClient client = server.makeDeviceClient();

        SceneObjectController controller = new SceneObjectController(cutObj, client);
        scene.addController(controller);

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
            driver.addModule(server);
            driver.addModule(viewer);
            driver.addModule(sceneManager);
            driver.setDesiredDt(0.001);

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
        SurfaceMesh clothMesh = makeClothGeometry(width, height, nRows, nCols);

        // Setup the Parameters
        PbdModelConfig pbdParams = new PbdModelConfig();
        pbdParams.enableConstraint(PbdModelConfig.ConstraintGenType.Distance, 1.0e3);
        pbdParams.enableConstraint(PbdModelConfig.ConstraintGenType.Dihedral, 1.0e3);
        pbdParams.m_fixedNodeIds = new VectorSizet(2);
        pbdParams.m_fixedNodeIds.Add(0);
        pbdParams.m_fixedNodeIds.Add((uint)colCount - 1);
        pbdParams.m_uniformMassValue = width * height / (rowCount * colCount);
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
        clothObj.setCollidingGeometry(clothMesh);
        clothObj.setDynamicalModel(pbdModel);

        return clothObj;
    }

    static SurfaceMesh makeClothGeometry(double width,
                                         double height,
                                         int    nRows,
                                         int    nCols)
    {
        SurfaceMesh clothMesh = new SurfaceMesh("Cloth_SurfaceMesh");

        VecDataArray3d vertices = new VecDataArray3d(nRows * nCols);
        double dy = width / (nCols - 1);
        double dx = height / (nRows - 1);
        for (int i = 0; i < nRows; ++i)
        {
            Vec3d xyz = new Vec3d();
            for (int j = 0; j < nCols; j++)
            {
                xyz[0] = dx * i;
                xyz[1] = 1.0;
                xyz[2] = dy * j;
                vertices[(uint)(i * nCols + j)] = xyz;
            }
        }

        // Add connectivity data
        VecDataArray3i indices = new VecDataArray3i();
        for (int i = 0; i < nRows - 1; i++)
        {
            for (int j = 0; j < nCols - 1; j++)
            {
                int index1 = i * nCols + j;
                int index2 = index1 + nCols;
                int index3 = index1 + 1;
                int index4 = index2 + 1;

                // Interleave [/][\]
                if (i % 2 != 0 || j % 2 != 0)
                {
                    indices.push_back(new Vec3i(index1, index2, index3));
                    indices.push_back(new Vec3i(index4, index3, index2));
                }
                else
                {
                    indices.push_back(new Vec3i(index2, index4, index1));
                    indices.push_back(new Vec3i(index4, index3, index1));
                }
            }
        }

        clothMesh.initialize(vertices, indices);

        return clothMesh;
    }
}