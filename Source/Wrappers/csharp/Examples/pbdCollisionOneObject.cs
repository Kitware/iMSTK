using imstk;

public class PbdCollisionOneObject
{
    public static void Main(string[] args)
    {
        // Write log to stdout and file
        Logger.startLogger();
        runPbdCollisionOneObject();
    }


    private static void runPbdCollisionOneObject()
    {
        // Setup the scene
        Scene scene = new Scene("PbdCollisionOneDragon");
        {
            scene.getActiveCamera().setPosition(0, 3.0, 20.0);
            scene.getActiveCamera().setFocalPoint(0.0, -10.0, 0.0);

            // set up the meshes
            string tetMeshFileName  = dataPath + "asianDragon/asianDragon.veg";
            TetrahedralMesh coarseTetMesh   = MeshIO.readTetrahedralMesh(tetMeshFileName);
            string surfMeshFileName = dataPath + "asianDragon/asianDragon.obj";
            // string surfMeshFileName = "/home/jianfeng/Documents/imstk/build_csharp/install/data/asianDragon/asianDragon.obj";
            SurfaceMesh highResSurfMesh = MeshIO.readSurfaceMesh(surfMeshFileName);
            SurfaceMesh coarseSurfMesh = coarseTetMesh.extractSurfaceMesh();

            // set up visual model based on high res mesh
            RenderMaterial material = new RenderMaterial();
            material.setDisplayMode(RenderMaterial.DisplayMode.Surface);
            material.setLineWidth(0.5f);
            material.setEdgeColor(Color.Blue);
            material.setShadingModel(RenderMaterial.ShadingModel.Phong);
            VisualModel surfMeshModel = new VisualModel(highResSurfMesh);
            surfMeshModel.setRenderMaterial(material);

            // configure the deformable object
            PbdObject deformableObj = new PbdObject("DeformableObj");
            deformableObj.addVisualModel(surfMeshModel);
            deformableObj.setCollidingGeometry(coarseSurfMesh);
            deformableObj.setPhysicsGeometry(coarseTetMesh);
            deformableObj.setPhysicsToCollidingMap(new OneToOneMap(coarseTetMesh, coarseSurfMesh));
            deformableObj.setPhysicsToVisualMap(new TetraTriangleMap(coarseTetMesh, highResSurfMesh));

            // Create model and object
            PbdModel pbdModel = new PbdModel();
            pbdModel.setModelGeometry(coarseTetMesh);

            // configure model
            PBDModelConfig pbdParams = new PBDModelConfig();

            // FEM constraint
            pbdParams.m_femParams.m_YoungModulus = youngModulus;
            pbdParams.m_femParams.m_PoissonRatio = poissonRatio;
            pbdParams.enableFEMConstraint(PbdConstraint.Type.FEMTet, PbdFEMConstraint.MaterialType.Corotation);

            // Other parameters
            // \todo use lumped mass
            pbdParams.m_uniformMassValue = 1.0;
            pbdParams.m_gravity    = new Vec3d(0, -10.0, 0);
            pbdParams.m_dt  = timeStep;
            pbdParams.m_iterations = maxIter;
            // pbdParams.collisionParams.m_proximity = 0.3;
            // pbdParams.collisionParams.m_stiffness = 0.1;

            pbdModel.configure(pbdParams);
            deformableObj.setDynamicalModel(pbdModel);

            scene.addSceneObject(deformableObj);

            // Build floor geometry
            SurfaceMesh floorMesh = createUniformSurfaceMesh(100.0, 100.0, 2, 2);

            RenderMaterial floorMaterial = new RenderMaterial();
            floorMaterial.setDisplayMode(RenderMaterial.DisplayMode.WireframeSurface);
            VisualModel floorVisualModel = new VisualModel(floorMesh);
            floorVisualModel.setRenderMaterial(floorMaterial);

            PbdObject floorObj = new PbdObject("Floor");
            floorObj.setCollidingGeometry(floorMesh);
            floorObj.setPhysicsGeometry(floorMesh);
            floorObj.addVisualModel(floorVisualModel);

            PbdModel floorPbdModel = new PbdModel();
            floorPbdModel.setModelGeometry(floorMesh);

            // configure model
            PBDModelConfig floorPbdParams = new PBDModelConfig();
            floorPbdParams.m_uniformMassValue = 0.0;
            floorPbdParams.m_iterations       = 0;
            // floorPbdParams.collisionParams.m_proximity = -0.1;

            // Set the parameters
            floorPbdModel.configure(floorPbdParams);
            floorObj.setDynamicalModel(floorPbdModel);

            scene.addSceneObject(floorObj);

            // Collision
            // scene.getCollisionGraph().addInteraction(Utils.makeObjectInteractionPair(deformableObj, floorObj,
                                                                                 // InteractionType.PbdObjToPbdObjCollision, CollisionDetection.Type.MeshToMeshBruteForce));
            PbdObjectCollision pbdInteraction = new PbdObjectCollision(deformableObj, floorObj, "PointSetToPlaneCD");
            MeshToMeshBruteForceCD cd = new MeshToMeshBruteForceCD();
            cd.setInputGeometryA(deformableObj.getCollidingGeometry());
            cd.setInputGeometryB(floorObj.getCollidingGeometry());
            pbdInteraction.setCollisionDetection(cd);
            pbdInteraction.getCollisionHandlingA().setInputCollisionData(cd.getCollisionData());

            scene.getCollisionGraph().addInteraction(pbdInteraction);
            scene.buildTaskGraph();
            scene.reset();

            // Light
            DirectionalLight light = new DirectionalLight();
            light.setFocalPoint(new Vec3d(5, -8, -5));
            light.setIntensity(1);
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

    private static SurfaceMesh createUniformSurfaceMesh(double width, double height, int nRows, int nCols)
    {
        double dy = width / (double)(nCols - 1);
        double dx = height / (double)(nRows - 1);

        VecDataArray3d vertices = new VecDataArray3d(nRows * nCols);

        for (int i = 0; i < nRows; ++i)
        {
            for (int j = 0; j < nCols; j++)
            {
                double y = (double)(dy * j);
                double x = (double)(dx * i);
                vertices[(uint)(i * nCols + j)] = new Vec3d(x - height * 0.5, -10.0, y - width * 0.5);
            }
        }

        // c. Add connectivity data
        VecDataArray3i triangles = new VecDataArray3i();
        for (int i = 0; i < nRows - 1; ++i)
        {
            for (int j = 0; j < nCols - 1; j++)
            {
                triangles.push_back(new Vec3i(i * nCols + j, i * nCols + j + 1, (i + 1) * nCols + j));
                triangles.push_back(new Vec3i((i + 1) * nCols + j + 1, (i + 1) * nCols + j, i * nCols + j + 1));
            }
        }

        SurfaceMesh surfMesh = new SurfaceMesh();
        surfMesh.initialize(vertices, triangles);

        return surfMesh;
    }

    private static string dataPath = "../data/";
    // parameters to play with
    private const double youngModulus     = 1000.0;
    private const double poissonRatio     = 0.3;
    private const double timeStep         = 0.01;
    private const double contactStiffness = 0.1;
    private const int    maxIter = 5;
}

