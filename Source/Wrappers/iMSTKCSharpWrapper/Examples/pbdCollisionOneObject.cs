using Imstk;

public class PbdCollisionOneObject
{
    private static string dataPath = "../data/";
    // parameters to play with
    private const double youngModulus = 1000.0;
    private const double poissonRatio = 0.3;
    private const double timeStep = 0.01;
    private const int maxIter = 5;

    public static void Main(string[] args)
    {
        // Write log to stdout and file
        Logger.startLogger();

        // Setup the scene
        Scene scene = new Scene("PbdCollisionOneDragon");
        {
            // Set the camera
            scene.getActiveCamera().setPosition(0.248534, 9.71495, 17.3109);
            scene.getActiveCamera().setFocalPoint(0.927986, 2.73914, 2.15692);
            scene.getActiveCamera().setViewUp(0.0092602, 0.911091, -0.412101);

            // Setup a tetrahedral pbd dragon object
            PbdObject deformableObj = new PbdObject("DeformableObj");
            {
                // set up the meshes
                string tetMeshFileName = dataPath + "asianDragon/asianDragon.vtu";
                TetrahedralMesh coarseTetMesh = MeshIO.readTetrahedralMesh(tetMeshFileName);

                string surfMeshFileName = dataPath + "asianDragon/asianDragon.obj";
                SurfaceMesh highResSurfMesh = MeshIO.readSurfaceMesh(surfMeshFileName);
                highResSurfMesh.translate(new Vec3d(0.0, 10.0, 0.0), Geometry.TransformType.ApplyToData);
                coarseTetMesh.translate(new Vec3d(0.0, 10.0, 0.0), Geometry.TransformType.ApplyToData);

                // set up visual model based on high res mesh
                RenderMaterial material = new RenderMaterial();
                material.setDisplayMode(RenderMaterial.DisplayMode.Surface);
                material.setLineWidth(0.5);
                material.setEdgeColor(Color.Blue);
                material.setShadingModel(RenderMaterial.ShadingModel.Phong);
                VisualModel surfMeshModel = new VisualModel();
                surfMeshModel.setGeometry(highResSurfMesh);
                surfMeshModel.setRenderMaterial(material);

                // Create model and object
                PbdSystem dynamicalModel = new PbdSystem();
                dynamicalModel.setModelGeometry(coarseTetMesh);

                // Configure model
                PbdModelConfig pbdParams = new PbdModelConfig();
                pbdParams.m_secParams.m_YoungModulus = youngModulus;
                pbdParams.m_secParams.m_PoissonRatio = poissonRatio;
                pbdParams.enableStrainEnergyConstraint(PbdStrainEnergyConstraint.MaterialType.Corotation);
                pbdParams.m_gravity = new Vec3d(0, -10.0, 0);
                pbdParams.m_dt = timeStep;
                pbdParams.m_iterations = maxIter;
                dynamicalModel.configure(pbdParams);

                deformableObj.addVisualModel(surfMeshModel);
                deformableObj.addComponentCollider().setGeometry(coarseTetMesh);
                deformableObj.setPhysicsGeometry(coarseTetMesh);
                deformableObj.setPhysicsToVisualMap(new PointToTetMap(coarseTetMesh, highResSurfMesh));
                deformableObj.setDynamicalModel(dynamicalModel);
                deformableObj.getPbdBody().uniformMassValue = 1.0;
            }
            scene.addSceneObject(deformableObj);

            // Setup the floor for it to fall on
            SceneObject floorObj = new SceneObject("Floor");
            {
                Plane floorGeom = new Plane(new Vec3d(0.0, 0.0, 0.0), new Vec3d(0.0, 1.0, 0.0));
                floorGeom.setWidth(100.0);

                floorObj.addComponentCollider().setGeometry(floorGeom);
                floorObj.setVisualGeometry(floorGeom);
            }
            scene.addSceneObject(floorObj);

            // Collision
            scene.addInteraction(new PbdObjectCollision(deformableObj, floorObj, "PointSetToPlaneCD"));

            // Light
            DirectionalLight light = new DirectionalLight();
            light.setFocalPoint(new Vec3d(5.0, -8.0, -5.0));
            light.setIntensity(1.0);
            scene.addLight("light", light);
        }

        // Run the simulation
        {
            // Setup a viewer to render
            VTKViewer viewer = new VTKViewer("Viewer");
            viewer.setActiveScene(scene);

            // Setup a scene manager to advance the scene
            SceneManager sceneManager = new SceneManager("Scene Manager");
            sceneManager.setActiveScene(scene);
            sceneManager.pause(); // Start simulation paused

            SimulationManager driver = new SimulationManager();
            driver.addModule(viewer);
            driver.addModule(sceneManager);

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

            driver.start();
        }
    }
}