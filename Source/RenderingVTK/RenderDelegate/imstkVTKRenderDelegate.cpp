/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkVTKRenderDelegate.h"
#include "imstkLogger.h"
#include "imstkPointSet.h"
#include "imstkRenderDelegateObjectFactory.h"
#include "imstkVisualModel.h"
#include "imstkVolumeRenderMaterial.h"

#include <vtkAbstractMapper.h>
#include <vtkActor.h>
#include <vtkImageReader2.h>
#include <vtkImageReader2Factory.h>
#include <vtkTexture.h>
#include <vtkTransform.h>

namespace imstk
{
VTKRenderDelegate::VTKRenderDelegate() :
    m_transform(vtkSmartPointer<vtkTransform>::New()),
    m_actor(nullptr),
    m_mapper(nullptr),
    m_visualModel(nullptr),
    m_material(nullptr)
{
}

void
VTKRenderDelegate::initialize(std::shared_ptr<VisualModel> model)
{
    CHECK(model != nullptr)
        << "VTKRenderDelegate was given null VisualModel";
    m_visualModel = model;
    CHECK(m_visualModel->getRenderMaterial() != nullptr)
        << "VTKRenderDelegate was given a VisualModel " << m_visualModel->getName()
        << " which does not have a material";
    m_material = m_visualModel->getRenderMaterial();

    // When render material is modified call materialModified -> updateRenderProperties()
    queueConnect<Event>(m_material, &RenderMaterial::modified,
        shared_from_this(), &VTKRenderDelegate::materialModified);

    // When the visual model is modified call visualModelModified
    queueConnect<Event>(m_visualModel, &VisualModel::modified,
        shared_from_this(), &VTKRenderDelegate::visualModelModified);

    init();
}

std::shared_ptr<VTKRenderDelegate>
VTKRenderDelegate::makeDelegate(std::shared_ptr<VisualModel> visualModel)
{
    return RenderDelegateObjectFactory::makeRenderDelegate(visualModel);
}

void
VTKRenderDelegate::update()
{
    // Then leave it up to subclasses to implement how to process the events
    processEvents();
}

void
VTKRenderDelegate::processEvents()
{
    std::shared_ptr<RenderMaterial> renderMaterial = m_visualModel->getRenderMaterial();

    // Only use the most recent event from respective sender
    std::list<Command> cmds;
    bool               contains[2] = { false, false };
    rforeachEvent([&](Command cmd)
        {
            if (cmd.m_event->m_sender == m_visualModel.get() && !contains[0])
            {
                cmds.push_back(cmd);
                contains[0] = true;
            }
            else if (cmd.m_event->m_sender == renderMaterial.get() && !contains[1])
            {
                cmds.push_back(cmd);
                contains[1] = true;
            }
        });
    // Now do each event in order recieved
    for (std::list<Command>::reverse_iterator i = cmds.rbegin(); i != cmds.rend(); i++)
    {
        i->invoke();
    }
}

void
VTKRenderDelegate::visualModelModified(Event* imstkNotUsed(e))
{
    // Remove all modified's from the old material
    disconnect(m_material, shared_from_this(), &RenderMaterial::modified);

    m_material = m_visualModel->getRenderMaterial(); // Update handle

    // Recieve events from new material
    queueConnect<Event>(m_material, &RenderMaterial::modified,
        shared_from_this(), &VTKRenderDelegate::materialModified);

    // Update our render properties
    updateRenderProperties();
}

vtkSmartPointer<vtkTexture>
VTKRenderDelegate::getVTKTexture(std::shared_ptr<Texture> texture)
{
    vtkNew<vtkImageReader2Factory> readerFactory;
    std::string                    fileName    = texture->getPath();
    auto                           imageReader = readerFactory->CreateImageReader2(fileName.c_str());

    //imageReader.TakeReference(readerFactory->CreateImageReader2(fileName.c_str()));
    imageReader->SetFileName(fileName.c_str());
    imageReader->Update();

    // Create texture
    vtkNew<vtkTexture> vtktexture;
    //vtktexture->UseSRGBColorSpaceOn();
    vtktexture->SetInputConnection(imageReader->GetOutputPort());

    return vtktexture;
}
} // namespace imstk