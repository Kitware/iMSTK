using System;

public class PbdCloth
{
     public class CSReceiverFunc : ReceiverFunc {
         public CSReceiverFunc(PbdObject pbdObject) {
             clothObj = pbdObject;
         }
         private PbdObject clothObj;
         public override void call(KeyEvent e) {
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
                 // std.shared_ptr<VecDataArray<unsigned char, 3>> scalars = std.dynamic_pointer_cast<VecDataArray<unsigned char, 3>>(imageData.getScalars());
                 VecDataArray3uc scalars = modIMSTK.castToVecDataArray3uc(imageData.getScalars());
                 if (scalars == null) {
                    Console.WriteLine("scalars = null");
                 } else {
                    Console.WriteLine("scalars.size() = {0}", scalars.size());
                 }

                 // Vec3uc scalarPtr = scalars.getPointer();
                 for (int i = 0; i < scalars.size(); i++)
                 {
                    // Console.WriteLine("i = {0}", i);
                     // scalarPtr[i] = (scalarPtr[i].cast<double>() * 0.8).cast<unsigned char>();
                     // scalarPtr[i] = (byte)((double)(scalarPtr[i])*0.8);
                     // scalars[(uint)i] = (byte)((double)(scalars[(uint)i])*0.8);
                     scalars[(uint)i][0] = (byte)((double)(scalars[(uint)i][0])*0.8);
                     scalars[(uint)i][1] = (byte)((double)(scalars[(uint)i][1])*0.8);
                     scalars[(uint)i][2] = (byte)((double)(scalars[(uint)i][2])*0.8);
                 }
                 Console.WriteLine("'h' key is pressed to change the render material...");
                 clothObj.getVisualModel(0).getRenderMaterial().getTexture(Texture.Type.Diffuse).postModified();
             }
         }
     }

     public class CSReceiverFunc2 : ReceiverFunc {
         public CSReceiverFunc2(Action<KeyEvent> action) {
             action_ = action;
         }
         public override void call(KeyEvent e) {
             action_(e);
         }
         private Action<KeyEvent> action_;
     }

    public static void Main(string[] args)
    {
        // Write log to stdout and file
        Logger.startLogger();

        RunPbdCloth();
    }

    public static PbdObject makeClothObj(string name, double width, double height, int rowCount, int colCount)
    {
        PbdObject clothObj = new PbdObject(name);

        // Setup the Geometry
        SurfaceMesh clothMesh = makeClothGeometry(10.0, 10.0, 16, 16, 2.0);

        // Setup the Parameters
        PBDModelConfig pbdParams = new PBDModelConfig();
        pbdParams.enableConstraint(PbdConstraint.Type.Distance, 1.0e2);
        pbdParams.enableConstraint(PbdConstraint.Type.Dihedral, 1.0e1);
        pbdParams.m_fixedNodeIds = new VectorSizet(2);
        pbdParams.m_fixedNodeIds.Add(0);
        pbdParams.m_fixedNodeIds.Add((uint)colCount - 1);
        pbdParams.m_uniformMassValue = width * height / (rowCount * colCount);
        pbdParams.m_gravity    = new Vec3d(0.0, -9.8, 0.0);
        pbdParams.m_defaultDt  = 0.005;
        pbdParams.m_iterations = 5;

        // Setup the Model
        PbdModel pbdModel = new PbdModel();
        pbdModel.setModelGeometry(clothMesh);
        pbdModel.configure(pbdParams);

        // Setup the VisualModel
        RenderMaterial material = new RenderMaterial();
        material.setBackFaceCulling(false);
        material.setDisplayMode(RenderMaterial.DisplayMode.Surface);
        // material.setDisplayMode(RenderMaterial.DisplayMode.Wireframe);
        material.setShadingModel(RenderMaterial.ShadingModel.PBR);

        setFleshTextures(material);
        // setFabricTextures(material);
        VisualModel visualModel = new VisualModel(clothMesh);
        visualModel.setRenderMaterial(material);

        // Setup the Object
        clothObj.addVisualModel(visualModel);
        clothObj.setPhysicsGeometry(clothMesh);
        clothObj.setDynamicalModel(pbdModel);

        return clothObj;
    }

    static SurfaceMesh makeClothGeometry(double width,
                                         double height,
                                         int    nRows,
                                         int    nCols,
                                         double uvScale)
    {
        SurfaceMesh clothMesh = new SurfaceMesh();

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

        VecDataArray2f uvCoords = new VecDataArray2f(nRows * nCols);
        for (uint i = 0; i < nRows; ++i)
        {
            for (uint j = 0; j < nCols; j++)
            {
                uvCoords[(uint)(i * nCols + j)] = new Vec2f((float)i / nRows, (float)j / nCols) * (float)uvScale;
            }
        }

        clothMesh.initialize(vertices, indices);
        clothMesh.setVertexTCoords("uvs", uvCoords);

        return clothMesh;
    }

    public static void RunPbdCloth()
    {
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
                MouseSceneControl mouseControl = new MouseSceneControl(viewer.getMouseDevice());
                mouseControl.setSceneManager(sceneManager);
                viewer.addControl(mouseControl);

                KeyboardSceneControl keyControl = new KeyboardSceneControl(viewer.getKeyboardDevice());
                keyControl.setSceneManager(new SceneManagerWeakPtr(sceneManager));
                keyControl.setModuleDriver(new ModuleDriverWeakPtr(driver));
                viewer.addControl(keyControl);
            }

            {
                CSReceiverFunc receiverFunc = new CSReceiverFunc(clothObj);
                modIMSTK.queueConnectKeyEvent(viewer.getKeyboardDevice(), modIMSTK.KeyboardDeviceClient_getKeyPress_cb, sceneManager, receiverFunc);
            }

            {
                Action<KeyEvent> receiverAction = (KeyEvent e) => {
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
                        // std.shared_ptr<VecDataArray<unsigned char, 3>> scalars = std.dynamic_pointer_cast<VecDataArray<unsigned char, 3>>(imageData.getScalars());
                        VecDataArray3uc scalars = modIMSTK.castToVecDataArray3uc(imageData.getScalars());

                        // Vec3uc scalarPtr = scalars.getPointer();
                        for (int i = 0; i < scalars.size(); i++)
                        {
                            // scalarPtr[i] = (scalarPtr[i].cast<double>() * 0.8).cast<unsigned char>();
                            // scalarPtr[i] = (byte)((double)(scalarPtr[i])*0.8);
                            // scalars[(uint)i] = (byte)((double)(scalars[(uint)i])*0.8);
                            // scalars[(uint)i][0] = (byte)((double)(scalars[(uint)i][0])*0.8);
                            // scalars[(uint)i][1] = (byte)((double)(scalars[(uint)i][1])*0.8);
                            // scalars[(uint)i][2] = (byte)((double)(scalars[(uint)i][2])*0.8);
                        }
                        Console.WriteLine("'h' key is pressed to change the render material...");
                        clothObj.getVisualModel(0).getRenderMaterial().getTexture(Texture.Type.Diffuse).postModified();
                    }
                };
                CSReceiverFunc2 receiverFunc = new CSReceiverFunc2(receiverAction);
                modIMSTK.queueConnectKeyEvent(viewer.getKeyboardDevice(), modIMSTK.KeyboardDeviceClient_getKeyPress_cb, sceneManager, receiverFunc);
            }

            driver.start();
        }

    }

    private static void setFabricTextures(RenderMaterial material)
    {
        // ImageData diffuseTex = MeshIO.readImageData("../../../install/data/textures/fabricDiffuse.jpg");
        ImageData diffuseTex = MeshIO.readImageData("../../../install/data/textures/fabricDiffuse.jpg");
        material.addTexture(new Texture(diffuseTex, Texture.Type.Diffuse));
        ImageData normalTex = MeshIO.readImageData("../../../install/data/textures/fabricNormal.jpg");
        material.addTexture(new Texture(normalTex, Texture.Type.Normal));
        ImageData ormTex = MeshIO.readImageData("../../../install/data/textures/fabricORM.jpg");
        material.addTexture(new Texture(ormTex, Texture.Type.ORM));
    }

    private static void setFleshTextures(RenderMaterial material)
    {
        ImageData diffuseTex = MeshIO.readImageData("../../../install/data/textures/fleshDiffuse.jpg");
        material.addTexture(new Texture(diffuseTex, Texture.Type.Diffuse));
        ImageData normalTex = MeshIO.readImageData("../../../install/data/textures/fleshNormal.jpg");
        material.addTexture(new Texture(normalTex, Texture.Type.Normal));
        ImageData ormTex = MeshIO.readImageData("../../../install/data/textures/fleshORM.jpg");
        material.addTexture(new Texture(ormTex, Texture.Type.ORM));
    }

}
