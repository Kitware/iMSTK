/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details. 
*/

using Imstk;

public enum Geom
{
    Dragon,
    Heart
}

public class Input
{
    public string meshFileName;
    public VectorSizet fixedNodeIds;
}

public class FeDeformable
{
    private const Geom geom = Geom.Heart;
    private static Input input;

    public static void Main(string[] args)
    {
        // Write log to stdout and file
        Logger.startLogger();

        input = new Input();
        if (geom == Geom.Dragon)
        {
            input.meshFileName = "../data/asianDragon/asianDragon.veg";
            input.fixedNodeIds = new VectorSizet(3);
            input.fixedNodeIds.Add(50);
            input.fixedNodeIds.Add(126);
            input.fixedNodeIds.Add(177);
        }
        else if (geom == Geom.Heart)
        {
            input.meshFileName = "../data/textured_organs/heart_volume.vtk";
            input.fixedNodeIds = new VectorSizet(13);
            input.fixedNodeIds.Add(75);
            input.fixedNodeIds.Add(82);
            input.fixedNodeIds.Add(84);
            input.fixedNodeIds.Add(94);
            input.fixedNodeIds.Add(95);
            input.fixedNodeIds.Add(105);
            input.fixedNodeIds.Add(110);
            input.fixedNodeIds.Add(124);
            input.fixedNodeIds.Add(139);
            input.fixedNodeIds.Add(150);
            input.fixedNodeIds.Add(161);
            input.fixedNodeIds.Add(171);
            input.fixedNodeIds.Add(350);
        }

        // Construct the scene
        Scene scene = new Scene("DeformableBodyFEM");
        {
            Camera cam = scene.getActiveCamera();
            cam.setPosition(0.0, 2.0, -25.0);
            cam.setFocalPoint(0.0, 0.0, 0.0);

            // Load a tetrahedral mesh
            TetrahedralMesh tetMesh = MeshIO.readTetrahedralMesh(input.meshFileName);
            // CHECK(tetMesh != nullptr) << "Could not read mesh from file.";

            // Scene object 1: fe-FeDeformableObject
            FeDeformableObject deformableObj = makeFEDeformableObject(tetMesh);
            scene.addSceneObject(deformableObj);

            // Scene object 2: Plane
            Plane planeGeom = new Plane();
            planeGeom.setWidth(40.0);
            planeGeom.setPosition(0.0, -8.0, 0.0);
            SceneObject planeObj = new SceneObject("Plane");
            planeObj.setVisualGeometry(planeGeom);
            Collider collider = new Collider("Collider");
            planeObj.addComponent(collider);
            // setCollidingGeometry(planeGeom);
            scene.addSceneObject(planeObj);

            // Light
            DirectionalLight light = new DirectionalLight();
            light.setFocalPoint(new Vec3d(5.0, -8.0, -5.0));
            light.setIntensity(1);
            scene.addLight("light", light);
        }

        // Run the simulation
        {
            // Setup a viewer to render in its own thread
            VTKViewer viewer = new VTKViewer("Viewer");
            viewer.setActiveScene(scene);

            // Setup a scene manager to advance the scene in its own thread
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

    private static FeDeformableObject makeFEDeformableObject(TetrahedralMesh tetMesh)
    {
        SurfaceMesh surfMesh = tetMesh.extractSurfaceMesh();
        // surfMesh.flipNormals();

        // Configure dynamic model
        FemDeformableBodyModel dynaModel = new FemDeformableBodyModel();
        FemModelConfig config = new FemModelConfig();
        config.m_fixedNodeIds = input.fixedNodeIds;
        dynaModel.configure(config);
        //dynaModel.configure(iMSTK_DATA_ROOT "/asianDragon/asianDragon.config");

        dynaModel.setTimeStepSizeType(TimeSteppingType.Fixed);
        dynaModel.setModelGeometry(tetMesh);
        BackwardEuler timeIntegrator = new BackwardEuler(0.01); // Create and add Backward Euler time integrator
        dynaModel.setTimeIntegrator(timeIntegrator);

        RenderMaterial mat = new RenderMaterial();
        mat.setDisplayMode(RenderMaterial.DisplayMode.WireframeSurface);
        mat.setPointSize(10.0f);
        mat.setLineWidth(2.0f);
        mat.setEdgeColor(Color.Orange);
        VisualModel surfMeshModel = new VisualModel();
        surfMeshModel.setGeometry(surfMesh);
        surfMeshModel.setRenderMaterial(mat);

        // Scene object 1: Dragon
        FeDeformableObject deformableObj = new FeDeformableObject("Dragon");
        deformableObj.addVisualModel(surfMeshModel);
        deformableObj.setPhysicsGeometry(tetMesh);
        // Map simulated geometry to visual
        deformableObj.setPhysicsToVisualMap(new PointwiseMap(tetMesh, surfMesh));
        deformableObj.setDynamicalModel(dynaModel);

        return deformableObj;
    }
}