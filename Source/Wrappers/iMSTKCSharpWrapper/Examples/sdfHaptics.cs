using System;
using imstk;

public class SdfHaptics
{
    public static void Main(string[] args)
    {
        Logger.startLogger();

        Scene scene = new Scene("SDFHaptics");
        SurfaceMesh  axesMesh = MeshIO.readSurfaceMesh("../data/axesPoly.vtk");
        ImageData    sdfImage = MeshIO.readImageData("../data/stanfordBunny/stanfordBunny_SDF.nii");
        SignedDistanceField sdf = new SignedDistanceField(sdfImage.cast((byte)Utils.IMSTK_DOUBLE));
        {
            scene.getActiveCamera().setPosition(-2.3, 23.81, 45.65);
            scene.getActiveCamera().setFocalPoint(9.41, 8.45, 5.76);

            CollidingObject bunnyObj = new CollidingObject("Bunny");
            {
                bunnyObj.setCollidingGeometry(sdf);

                SurfaceMeshFlyingEdges isoExtract = new SurfaceMeshFlyingEdges();
                isoExtract.setInputImage(sdfImage);
                isoExtract.update();

                isoExtract.getOutputMesh().flipNormals();
                bunnyObj.setVisualGeometry(isoExtract.getOutputMesh());

                scene.addSceneObject(bunnyObj);
            }

            SceneObject axesObj = new SceneObject("Axes");
            {
                axesObj.setVisualGeometry(axesMesh);
                scene.addSceneObject(axesObj);
            }

            // Light (white)
            DirectionalLight whiteLight = new DirectionalLight();
            {
                whiteLight.setDirection(new Vec3d(5.0, -8.0, -5.0));
                whiteLight.setIntensity(1.0);
                scene.addLight("whiteLight", whiteLight);
            }
        }

        HapticDeviceManager hapticManager = new HapticDeviceManager();
        HapticDeviceClient client = hapticManager.makeDeviceClient();

        // Run the simulation
        {
            // Setup a viewer to render in its own thread
            VTKViewer viewer = new VTKViewer("Viewer");
            viewer.setActiveScene(scene);

            // Setup a scene manager to advance the scene in its own thread
            SceneManager sceneManager = new SceneManager("Scene Manager");
            sceneManager.setActiveScene(scene);
            sceneManager.setExecutionType(Module.ExecutionType.ADAPTIVE);

            SimulationManager driver = new SimulationManager();
            driver.addModule(viewer);
            driver.addModule(sceneManager);
            driver.addModule(hapticManager);

            ImplicitFunctionCentralGradient centralGrad = new ImplicitFunctionCentralGradient();
            centralGrad.setFunction(sdf);
            centralGrad.setDx(sdf.getImage().getSpacing());

            Utils.connectEvent(sceneManager, Utils.SceneManager_getPostUpdate_cb,
                (Event e) =>
                {
                    Vec3d tmpPos = client.getPosition();
                    Vec3d pos = tmpPos * 0.1 + new Vec3d(0.0, 0.1, 10.0);

                    client.update();
                    axesMesh.setTranslation(pos);
                    axesMesh.setRotation(client.getOrientation());
                    axesMesh.postModified();

                    double dx = sdf.getFunctionValue(pos);
                    if (dx < 0.0)
                    {
                        Vec3d g = centralGrad.compute(pos);
                        Vec3d nrm_g = new Vec3d(-g[0] * dx * 4.0, -g[1] * dx * 4.0, -g[2] * dx * 4.0);
                        client.setForce(nrm_g);
                    }
                });

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
