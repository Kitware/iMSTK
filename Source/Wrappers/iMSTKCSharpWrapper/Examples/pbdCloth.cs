using System;
using Imstk;

public class PbdCloth
{
    private static string dataPath = "../data/";

    public static void Main(string[] args)
    {
        // Write log to stdout and file
        Logger.startLogger();

        // Setup a scene
        Scene scene = new Scene("PBDCloth");
        PbdObject clothObj = makeClothObj("Cloth", 10.0, 10.0, 16, 16);
        scene.addSceneObject(clothObj);

        // Adjust camera
        scene.getActiveCamera().setFocalPoint(0.0, -5.0, 5.0);
        scene.getActiveCamera().setPosition(-15.0, -5.0, 25.0);

        // Run the simulation
        {
            // Setup a viewer to render
            VTKViewer viewer = new VTKViewer("Viewer");
            viewer.setActiveScene(scene);

            // Setup a scene manager to advance the scene
            SceneManager sceneManager = new SceneManager("Scene Manager");
            sceneManager.setExecutionType(Module.ExecutionType.ADAPTIVE);
            sceneManager.setActiveScene(scene);
            sceneManager.pause(); // Start simulation paused

            SimulationManager driver = new SimulationManager();
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
                    // Set new textures
                    if (e.m_key == '1')
                    {
                        setFleshTextures(clothObj.getVisualModel(0).getRenderMaterial());
                    }
                    else if (e.m_key == '2')
                    {
                        setFabricTextures(clothObj.getVisualModel(0).getRenderMaterial());
                    }
                    // Darken the texture pixel values
                    else if (e.m_key == 'h')
                    {
                        ImageData imageData = clothObj.getVisualModel(0).getRenderMaterial().getTexture(Texture.Type.Diffuse).getImageData();
                        VecDataArray3uc scalars = Utils.CastTo<VecDataArray3uc>(imageData.getScalars());
                        byte[] newScalars = new byte[3 * scalars.size()];
                        scalars.getValues(newScalars);

                        for (int i = 0; i < newScalars.Length; i++)
                        {
                            newScalars[i] = (byte)(newScalars[i] * 0.8);
                        }
                        scalars.setValues(newScalars);
                        clothObj.getVisualModel(0).getRenderMaterial().getTexture(Texture.Type.Diffuse).postModified();
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
            new Vec2d(10.0, 10.0), new Vec2i(16, 16), new Quatd(0.0, 0.0, 0.0, 1.0), 2.0);

        // Setup the Parameters
        PbdModelConfig pbdParams = new PbdModelConfig();
        pbdParams.enableConstraint(PbdModelConfig.ConstraintGenType.Distance, 1.0e2);
        pbdParams.enableConstraint(PbdModelConfig.ConstraintGenType.Dihedral, 1.0e1);
        pbdParams.m_gravity    = new Vec3d(0.0, -9.8, 0.0);
        pbdParams.m_dt  = 0.005;
        pbdParams.m_iterations = 5;

        // Setup the Model
        PbdSystem dynamicalModel = new PbdSystem();
        dynamicalModel.setModelGeometry(clothMesh);
        dynamicalModel.configure(pbdParams);

        // Setup the VisualModel
        RenderMaterial material = new RenderMaterial();
        material.setBackFaceCulling(false);
        material.setDisplayMode(RenderMaterial.DisplayMode.Surface);
        // material.setDisplayMode(RenderMaterial.DisplayMode.Wireframe);
        material.setShadingModel(RenderMaterial.ShadingModel.PBR);

        setFleshTextures(material);
        // setFabricTextures(material);
        VisualModel visualModel = new VisualModel();
        visualModel.setGeometry(clothMesh);
        visualModel.setRenderMaterial(material);

        // Setup the Object
        clothObj.addVisualModel(visualModel);
        clothObj.setPhysicsGeometry(clothMesh);
        clothObj.setDynamicalModel(dynamicalModel);

        clothObj.getPbdBody().fixedNodeIds = new VectorInt(2);
        clothObj.getPbdBody().fixedNodeIds.Add(0);
        clothObj.getPbdBody().fixedNodeIds.Add(colCount - 1);
        clothObj.getPbdBody().uniformMassValue = width * height / (rowCount * colCount);

        return clothObj;
    }

    private static void setFabricTextures(RenderMaterial material)
    {
        ImageData diffuseTex = MeshIO.readImageData(dataPath + "textures/fabricDiffuse.jpg");
        material.addTexture(new Texture(diffuseTex, Texture.Type.Diffuse));
        ImageData normalTex = MeshIO.readImageData(dataPath + "textures/fabricNormal.jpg");
        material.addTexture(new Texture(normalTex, Texture.Type.Normal));
        ImageData ormTex = MeshIO.readImageData(dataPath + "textures/fabricORM.jpg");
        material.addTexture(new Texture(ormTex, Texture.Type.ORM));
    }

    private static void setFleshTextures(RenderMaterial material)
    {
        ImageData diffuseTex = MeshIO.readImageData(dataPath + "textures/fleshDiffuse.jpg");
        material.addTexture(new Texture(diffuseTex, Texture.Type.Diffuse));
        ImageData normalTex = MeshIO.readImageData(dataPath + "textures/fleshNormal.jpg");
        material.addTexture(new Texture(normalTex, Texture.Type.Normal));
        ImageData ormTex = MeshIO.readImageData(dataPath + "textures/fleshORM.jpg");
        material.addTexture(new Texture(ormTex, Texture.Type.ORM));
    }
}
