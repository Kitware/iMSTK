using System;
using Imstk;

public class RigidBody2
{
    public static void Main(string[] args)
    {
        // Write log to stdout and file
        Logger.startLogger();

        Scene scene = new Scene("Rigid Body Dynamics");
        RigidObject2 cubeObj = new RigidObject2("Cube");
        {
            // This model is shared among interacting rigid bodies
            RigidBodyModel2 rbdModel = new RigidBodyModel2();
            rbdModel.getConfig().m_gravity = new Vec3d(0.0, -2500.0, 0.0);
            rbdModel.getConfig().m_maxNumIterations = 10;

            // Create the first rbd, plane floor
            CollidingObject planeObj = new CollidingObject("Plane");
            {
                // Subtract the sphere from the plane to make a crater
                Plane planeGeom = new Plane();
                planeGeom.setWidth(40.0);
                Sphere sphereGeom = new Sphere();
                sphereGeom.setRadius(25.0);
                sphereGeom.setPosition(0.0, 10.0, 0.0);
                CompositeImplicitGeometry compGeom = new CompositeImplicitGeometry();
                compGeom.addImplicitGeometry(planeGeom, CompositeImplicitGeometry.GeometryBoolType.Union);
                compGeom.addImplicitGeometry(sphereGeom, CompositeImplicitGeometry.GeometryBoolType.Difference);

                // Rasterize the SDF into an image
                ImplicitGeometryToImageData toImage = new ImplicitGeometryToImageData();
                toImage.setInputGeometry(compGeom);
                Vec6d bounds = new Vec6d();
                bounds[0] = -20.0;
                bounds[1] = 20.0;
                bounds[2] = -20.0;
                bounds[3] = 20.0;
                bounds[4] = -20.0;
                bounds[5] = 20.0;
                toImage.setBounds(bounds);
                toImage.setDimensions(new Vec3i(80, 80, 80));
                toImage.update();

                // Extract surface
                SurfaceMeshFlyingEdges toSurfMesh = new SurfaceMeshFlyingEdges();
                toSurfMesh.setInputImage(toImage.getOutputImage());
                toSurfMesh.update();
                toSurfMesh.getOutputMesh().flipNormals();

                // Create the visual model
                VisualModel visualModel = new VisualModel();
                visualModel.setGeometry(toSurfMesh.getOutputMesh());

                // Create the object
                planeObj.addVisualModel(visualModel);
                planeObj.setCollidingGeometry(compGeom);
                //planeObj.getRigidBody().m_isStatic = true;
                //planeObj.getRigidBody().m_mass     = 100.0;

                scene.addSceneObject(planeObj);
            }

            // Create surface mesh cube (so we can use pointset for point.implicit collision)
            {
                OrientedBox cubeGeom = new OrientedBox(new Vec3d(0.0, 0.0, 0.0), new Vec3d(1.5, 3.0, 1.0));
                SurfaceMesh surfMesh = Utils.toSurfaceMesh(cubeGeom);

                SurfaceMeshSubdivide subdivide = new SurfaceMeshSubdivide();
                subdivide.setInputMesh(surfMesh);
                subdivide.setNumberOfSubdivisions(1);
                subdivide.update();

                // Create the visual model
                VisualModel visualModel = new VisualModel();
                visualModel.setGeometry(subdivide.getOutputMesh());
                RenderMaterial mat = new RenderMaterial();
                mat.setDisplayMode(RenderMaterial.DisplayMode.WireframeSurface);
                mat.setLineWidth(2.0f);
                mat.setColor(Color.Orange);
                visualModel.setRenderMaterial(mat);

                // Create the cube rigid object
                cubeObj.setDynamicalModel(rbdModel);
                cubeObj.setPhysicsGeometry(subdivide.getOutputMesh());
                cubeObj.setCollidingGeometry(subdivide.getOutputMesh());
                cubeObj.addVisualModel(visualModel);
                cubeObj.getRigidBody().m_mass = 100.0;
                cubeObj.getRigidBody().m_initPos = new Vec3d(0.0, 8.0, 0.0);
                Rotd rotd = new Rotd(0.4, new Vec3d(1.0, 0.0, 0.0));
                cubeObj.getRigidBody().m_initOrientation = new Quatd(new Rotd(0.4, new Vec3d(1.0, 0.0, 0.0)));
                cubeObj.getRigidBody().m_intertiaTensor = Mat3d.Identity();

                scene.addSceneObject(cubeObj);
            }

            RigidObjectCollision rbdInteraction = new RigidObjectCollision(cubeObj, planeObj, "ImplicitGeometryToPointSetCD");
            rbdInteraction.setFriction(0.0);
            rbdInteraction.setBaumgarteStabilization(0.05);
            scene.addInteraction(rbdInteraction);
            scene.getActiveCamera().setPosition(0.0, 40.0, 40.0);

            // Light
            DirectionalLight light = new DirectionalLight();
            light.setIntensity(1.0);
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
            sceneManager.setExecutionType(Module.ExecutionType.ADAPTIVE);
            sceneManager.pause();

            SimulationManager driver = new SimulationManager();
            driver.addModule(viewer);
            driver.addModule(sceneManager);
            driver.setDesiredDt(0.001);

            // Add mouse and keyboard controls to the viewer
            {
                MouseSceneControl mouseControl = new MouseSceneControl(viewer.getMouseDevice());
                mouseControl.setSceneManager(sceneManager);
                scene.addControl(mouseControl);

                KeyboardSceneControl keyControl = new KeyboardSceneControl(viewer.getKeyboardDevice());
                keyControl.setSceneManager(new SceneManagerWeakPtr(sceneManager));
                keyControl.setModuleDriver(new ModuleDriverWeakPtr(driver));
                scene.addControl(keyControl);
            }

            // LOG(INFO) << "Cube Controls:";
            // LOG(INFO) << "----------------------------------------------------------------------";
            // LOG(INFO) << " | i - forward movement";
            // LOG(INFO) << " | j - left movement";
            // LOG(INFO) << " | l - right movement";
            // LOG(INFO) << " | k - backwards movement";
            // LOG(INFO) << " | u - rotate left";
            // LOG(INFO) << " | o - rotate right";


            Vec3d dx = Utils.vec_subtract_3d(scene.getActiveCamera().getPosition(), scene.getActiveCamera().getFocalPoint());
            KeyboardDeviceClient keyDevice = viewer.getKeyboardDevice();
            Utils.connectEvent(sceneManager, Utils.SceneManager_getPostUpdate_cb,
                (Event e) =>
                {
                    Vec3d extForce = new Vec3d(0.0, 0.0, 0.0);
                    Vec3d extTorque = new Vec3d(0.0, 0.0, 0.0);
                    // If w down, move forward
                    if (keyDevice.getButton('i') == 1)
                    {
                        extForce = Utils.vec_add_3d(extForce, new Vec3d(0.0, 0.0, -900.0));
                    }
                    if (keyDevice.getButton('k') == 1)
                    {
                        extForce = Utils.vec_add_3d(extForce, new Vec3d(0.0, 0.0, 900.0));
                    }
                    if (keyDevice.getButton('j') == 1)
                    {
                        extForce = Utils.vec_add_3d(extForce, new Vec3d(-900.0, 0.0, 0.0));
                    }
                    if (keyDevice.getButton('l') == 1)
                    {
                        extForce = Utils.vec_add_3d(extForce, new Vec3d(900.0, 0.0, 0.0));
                    }
                    if (keyDevice.getButton('u') == 1)
                    {
                        extForce = Utils.vec_add_3d(extForce, new Vec3d(0.0, 1.5, 0.0));
                    }
                    if (keyDevice.getButton('o') == 1)
                    {
                        extForce = Utils.vec_add_3d(extForce, new Vec3d(0.0, -1.5, 0.0));
                    }
                    // \todo: Add setters to imstk
                    cubeObj.getRigidBody().m_force = extForce;
                    cubeObj.getRigidBody().m_torque = extTorque;
                    scene.getActiveCamera().setFocalPoint(cubeObj.getRigidBody().getPosition());
                    scene.getActiveCamera().setPosition(Utils.vec_add_3d(cubeObj.getRigidBody().getPosition(), dx));
                    cubeObj.getRigidBodyModel2().getConfig().m_dt = sceneManager.getDt();
                });

            driver.start();
        }
    }
}