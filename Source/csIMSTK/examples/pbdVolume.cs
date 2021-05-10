using System;

public class PbdVolume
{
    public static void Main(string[] args)
    {
        // Write log to stdout and file
        Logger.startLogger();
        runPbdDeformable();
    }

    private static PbdObject createAndAddPbdObject(string tetMeshName)
    {
        TetrahedralMesh tetMesh = MeshIO.readTetrahedralMesh(tetMeshName);
        tetMesh.rotate(new Vec3d(1.0, 0.0, 0.0), -1.3, Geometry.TransformType.ApplyToData);
        SurfaceMesh surfMesh = new SurfaceMesh();
        tetMesh.extractSurfaceMesh(surfMesh, true);
        surfMesh.flipNormals();

        RenderMaterial material = new RenderMaterial();
        material.setDisplayMode(RenderMaterial.DisplayMode.Surface);
        material.setColor(new Color(220.0 / 255.0, 100.0 / 255.0, 70.0 / 255.0));
        material.setMetalness(100.9f);
        material.setRoughness(0.5f);
        material.setEdgeColor(Color.Teal);
        material.setShadingModel(RenderMaterial.ShadingModel.Phong);
        material.setDisplayMode(RenderMaterial.DisplayMode.WireframeSurface);
        VisualModel visualModel = new VisualModel(surfMesh);
        visualModel.setRenderMaterial(material);

        PbdObject deformableObj = new PbdObject("DeformableObject");
        PbdModel pbdModel = new PbdModel();
        pbdModel.setModelGeometry(tetMesh);

        // Configure model
        PBDModelConfig pbdParams = new PBDModelConfig();

        // FEM constraint
        pbdParams.m_femParams.m_YoungModulus = 500.0;
        pbdParams.m_femParams.m_PoissonRatio = 0.3;
        pbdParams.m_fixedNodeIds = new VectorSizet(13);
        pbdParams.m_fixedNodeIds.Add(75);
        pbdParams.m_fixedNodeIds.Add(82);
        pbdParams.m_fixedNodeIds.Add(84);
        pbdParams.m_fixedNodeIds.Add(94);
        pbdParams.m_fixedNodeIds.Add(95);
        pbdParams.m_fixedNodeIds.Add(105);
        pbdParams.m_fixedNodeIds.Add(110);
        pbdParams.m_fixedNodeIds.Add(124);
        pbdParams.m_fixedNodeIds.Add(139);
        pbdParams.m_fixedNodeIds.Add(150);
        pbdParams.m_fixedNodeIds.Add(161);
        pbdParams.m_fixedNodeIds.Add(171);
        pbdParams.m_fixedNodeIds.Add(350);
        pbdParams.enableFEMConstraint(PbdConstraint.Type.FEMTet, PbdFEMConstraint.MaterialType.StVK);

        // Other parameters
        pbdParams.m_uniformMassValue = 1.0;
        pbdParams.m_gravity    = new Vec3d(0, -9.8, 0);
        pbdParams.m_iterations = 6;

        // Set the parameters
        pbdModel.configure(pbdParams);
        pbdModel.setDefaultTimeStep(0.02);
        pbdModel.setTimeStepSizeType(TimeSteppingType.Fixed);

        deformableObj.setDynamicalModel(pbdModel);
        deformableObj.addVisualModel(visualModel);
        deformableObj.setPhysicsGeometry(tetMesh);
        deformableObj.setPhysicsToVisualMap(new TetraTriangleMap(tetMesh, surfMesh));

        return deformableObj;
    }

    private static void runPbdDeformable()
    {
        Scene scene = new Scene("PBDVolume");
        scene.getActiveCamera().setPosition(0, 2.0, 15.0);

        string tetMeshFileName = "../../../install/data/textured_organs/heart_volume.vtk";
        // create and add a PBD object
        scene.addSceneObject(createAndAddPbdObject(tetMeshFileName));

        // Light
        DirectionalLight light = new DirectionalLight("light");
        light.setFocalPoint(new Vec3d(5, -8, -5));
        light.setIntensity(1.1);
        scene.addLight(light);

        // Run the simulation
        {
            // Setup a viewer to render
            VTKViewer viewer = new VTKViewer("Viewer");
            viewer.setActiveScene(scene);
            viewer.setBackgroundColors(new Color(0.3285, 0.3285, 0.6525), new Color(0.13836, 0.13836, 0.2748), true);

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
}
