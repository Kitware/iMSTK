using System;
using Imstk;

public class PbdCloth
{
    public static void Main(string[] args)
    {
        // Write log to stdout and file
        Logger.startLogger();

        Capsule capsule = new Capsule(new Vec3d(0.0, -4.0, 0.0), 2.0, 5.0, new Quatd(new Rotd(1.5708, new Vec3d(0.0, 0.0, 1.0))));
        Sphere sphere = new Sphere(new Vec3d(0.0, -2.0, 0.0), 2.0);
        OrientedBox cube = new OrientedBox(new Vec3d(0.0, -4.0, 0.0), new Vec3d(2.5, 2.5, 2.5));
        Plane plane = new Plane(new Vec3d(0.0, -2.0, 0.0), new Vec3d(0.0, 1.0, 0.0));
        plane.setWidth(20.0);

        Geometry[] geometries = new Geometry[] { capsule, sphere, cube, plane };

        // Setup a scene
        Scene scene = new Scene("PBDClothCollision");
        PbdObject clothObj = makeClothObj("Cloth", 10.0, 10.0, 16, 16);
        scene.addSceneObject(clothObj);
        CollidingObject collisionObj = new CollidingObject("CollidingObject");
        collisionObj.setCollidingGeometry(capsule);

        for (int i = 0; i < 4; ++i)
        {
            VisualModel visualModel = new VisualModel(geometries[i]);
            visualModel.getRenderMaterial().setBackFaceCulling(false);
            visualModel.getRenderMaterial().setOpacity(0.5);
            visualModel.hide();
            collisionObj.addVisualModel(visualModel);
        }
        collisionObj.getVisualModel(0).show();
        scene.addSceneObject(collisionObj);

        PbdObjectCollision pbdInteraction = new PbdObjectCollision(clothObj, collisionObj, "PointSetToCapsuleCD");
        pbdInteraction.setFriction(0.4);
        pbdInteraction.setRestitution(0.0);
        scene.getCollisionGraph().addInteraction(pbdInteraction);


        // Adjust camera
        scene.getActiveCamera().setFocalPoint(0.0, -2.0, 0.0);
        scene.getActiveCamera().setPosition(5.0, 4.0, 18.0);

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

            Utils.connectKeyEvent(viewer.getKeyboardDevice(), Utils.KeyboardDeviceClient_getKeyPress_cb,
                (KeyEvent e) =>
                {
                    // Switch to sphere and reset
                    int indexToShow = -1;
                    CollisionDetectionAlgorithm newCDMethod = null;
                    if (e.m_key == '1')
                    {
                        indexToShow = 0;
                        newCDMethod = new PointSetToCapsuleCD();
                    }
                    // Switch to capsule and reset
                    else if (e.m_key == '2')
                    {
                        indexToShow = 1;
                        newCDMethod = new PointSetToSphereCD();
                    }
                    // Switch to cube and reset
                    else if (e.m_key == '3')
                    {
                        indexToShow = 2;
                        newCDMethod = new PointSetToOrientedBoxCD();
                    }
                    // Switch to plane and reset
                    else if (e.m_key == '4')
                    {
                        indexToShow = 3;
                        newCDMethod = new PointSetToPlaneCD();
                    }
                    // Switch to sphere vs surface and reset
                    else if (e.m_key == '5')
                    {
                        indexToShow = 1;
                        newCDMethod = new SurfaceMeshToSphereCD();
                    }
                    // Switch to sphere vs surface and reset
                    else if (e.m_key == '6')
                    {
                        indexToShow = 0;
                        newCDMethod = new SurfaceMeshToCapsuleCD();
                    }

                    if (indexToShow != -1)
                    {
                        // Hide all models
                        for (uint i = 0; i < 4; i++)
                        {
                            collisionObj.getVisualModel(i).hide();
                        }
                        // Show the selected one
                        VisualModel visualModel = collisionObj.getVisualModel((uint)indexToShow);
                        visualModel.show();
                        collisionObj.setCollidingGeometry(visualModel.getGeometry());

                        newCDMethod.setInputGeometryA(clothObj.getCollidingGeometry());
                        newCDMethod.setInputGeometryB(visualModel.getGeometry());
                        pbdInteraction.setCollisionDetection(newCDMethod);
                        pbdInteraction.getCollisionHandlingA().setInputCollisionData(newCDMethod.getCollisionData());

                        scene.buildTaskGraph();
                        scene.initTaskGraph();
                        scene.reset();
                    }
                });

            driver.start();
        }

    }

    public static PbdObject makeClothObj(string name, double width, double height, int rowCount, int colCount)
    {
        PbdObject clothObj = new PbdObject(name);

        // Setup the Geometry
        SurfaceMesh clothMesh = makeClothGeometry(width, height, rowCount, colCount, 2.0);

        // Setup the Parameters
        PbdModelConfig pbdParams = new PbdModelConfig();
        pbdParams.enableConstraint(PbdModelConfig.ConstraintGenType.Distance, 1.0e2);
        pbdParams.enableConstraint(PbdModelConfig.ConstraintGenType.Dihedral, 1.0e1);
        pbdParams.m_uniformMassValue = width * height / (rowCount * colCount);
        pbdParams.m_gravity = new Vec3d(0.0, -9.8, 0.0);
        pbdParams.m_dt = 0.005;
        pbdParams.m_iterations = 5;

        // Setup the Model
        PbdModel pbdModel = new PbdModel();
        pbdModel.setModelGeometry(clothMesh);
        pbdModel.configure(pbdParams);

        // Setup the VisualModel
        RenderMaterial material = new RenderMaterial();
        material.setBackFaceCulling(false);
        material.setDisplayMode(RenderMaterial.DisplayMode.WireframeSurface);
        material.setColor(Color.Blue);

        // setFabricTextures(material);
        VisualModel visualModel = new VisualModel(clothMesh);
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
                                         int nRows,
                                         int nCols,
                                         double uvScale)
    {
        SurfaceMesh clothMesh = new SurfaceMesh();

        VecDataArray3d vertices = new VecDataArray3d(nRows * nCols);
        double dy = width / (nCols - 1);
        double dx = height / (nRows - 1);
        Vec3d halfSize = new Vec3d(height, 0.0, width) * 0.5;
        for (int i = 0; i < nRows; ++i)
        {
            Vec3d xyz = new Vec3d();
            for (int j = 0; j < nCols; j++)
            {
                xyz[0] = dx * i;
                xyz[1] = 0.05;
                xyz[2] = dy * j - 1.0;
                vertices[(uint)(i * nCols + j)] = xyz - halfSize;
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
}