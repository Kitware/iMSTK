Overview
========

VTK is currently the primary rendering backend for iMSTK but iMSTK may also be used standalone, without rendering. This also allows one to utilize any rendering backend they wish.

A VTKViewer is used to render an iMSTK Scene. It can be setup as:

::

    imstkNew<VTKViewer> viewer;
    viewer->setActiveScene(myScene);

This sets up a imstk VTKRenderer to render that scene. What the VTKRenderer does is go through all of the VisualModels of every SceneObject in the Scene and setup a RenderDelegate.

While a SceneObject knows nothing of VTK, a VTKRenderDelegate will setup a VTK dataset, mappers, and actors for rendering in VTK. Ideally we will avoid copying the dataset and instead couple it with VTK so they point to same data.

Typically the SimulationManager or another ModuleDriver will drive the VTKViewer and SceneManager. But if one would instead like to invoke render themselves. They simply need to call update on the viewer.

::

    viewer->update(); // Renders now

Render Materials
================

The default RenderMaterial contains many rendering parameters for an object. Some may be consumed, some may not. Importantly we have a DisplayMode

- Surface: Renders only the surface of the geometry.
- Wireframe: Renders only the wireframe of the geometry, edges of the cells.
- Points: Renders only the vertices of the geometry.
- WireframeSurface: Renders both the surface of the cells

You then have a choice between ShadingModels
- Phong: Computes lighting with normals per every fragments/pixels.
- Gourand: Computes lighting with normals per vertex then interpolates over fragments/pixels.
- Flat: Computes lighting per cell and uses it for every fragment/pixel of that cell.
- PBR: Uses phong shading but has a very specific model based on realistic parameters.

To setup a very basic material we can do:

::

    imstkNew<RenderMaterial> mat;
    mat->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    mat->setLineWidth(2.0);
    mat->setEdgeColor(Color::Orange);

This gives us orange edges, gray default surface color, and edges that are displayed with size 2. The typical phong model allows us to specify specular, diffuse, and ambient lighting. We give scales and colors for each.

Alternatively we can use PBR which is parameterized differently based on realistic parameters such as "roughness" or "metalness". There are also a number of other features. Here we show how to use that with textures for diffuse, normals, and ambient occlusion.

::

    imstkNew<RenderMaterial> material;
    material->setDisplayMode(RenderMaterial::DisplayMode::Surface);
    material->setShadingModel(RenderMaterial::ShadingModel::PBR);
    imstkNew<Texture> headDiffuseTexture(iMSTK_DATA_ROOT "head/HeadTexture_BaseColor.png", Texture::Type::Diffuse);
    imstkNew<Texture> headNormalTexture(iMSTK_DATA_ROOT "head/HeadTexture_Normal.png", Texture::Type::Normal);
    imstkNew<Texture> headAoTexture(iMSTK_DATA_ROOT "head/HeadTexture_AO.png", Texture::Type::AmbientOcclusion;

    material->setRoughness(10.0);
    material->setMetalness(0.0);
    material->addTexture(headDiffuseTexture);
    material->addTexture(headNormalTexture);
    material->addTexture(headAoTexture);

Another important function is setRecomputeVertexNormals. With this the per vertex normals of a mesh (which are used for lighting) are recomputed before rendering. This is particularly useful for dynamic objects or animations. Without it VTK shaders will rigidly transform the normals for you. This is more performant for static or rigid objects, but for deformable objects we must compute them ourselves every time. By default it is on for any dynamic object.

VolumeRenderMaterial
====================

The VolumeRenderMaterial exposes VTK objects for rendering, it can be used as:

::

    imstkNew<VolumeRenderMaterial>   volumeMaterial;
    vtkNew<vtkColorTransferFunction> colorFunc;
    colorFunc->AddRGBPoint(0.0, 1.0, 0.0, 0.0);
    colorFunc->AddRGBPoint(8.0, 0.0, 0.0, 1.0);
    volumeMaterial->getVolumeProperty()->SetColor(colorFunc);
    vtkNew<vtkPiecewiseFunction> opacityFunc;
    opacityFunc->AddPoint(0.0, 0.0);
    opacityFunc->AddPoint(1.0, 1.0);
    volumeMaterial->getVolumeProperty()->SetScalarOpacity(opacityFunc);

Here we set both a color and opacity function. As we render the volume (by marching along a ray shot from the screen) we sample the image we are rendering and lookup a color and opacity. These functions specify this. AddRGBPoint(intensity, r, g, b) and AddPoint(intensity, opacity).

A number of presets (which may or may not fit your image/volume) are also available which can be used as below:

::

    std::shared_ptr<VolumeRenderMaterial> volumeMaterial =
	VolumeRenderMaterialPresets::getPreset(currMatId);