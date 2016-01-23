// This file is part of the SimMedTK project.
// Copyright (c) Kitware, Inc.
//
// Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
//                        Rensselaer Polytechnic Institute
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef RENDERING_VTKRENDERDELEGATE_H
#define RENDERING_VTKRENDERDELEGATE_H

#include <vtkOpenGLPolyDataMapper.h>
#include "Core/RenderDelegate.h"
//#include "Core/RenderDetail.h"

class vtkActor;
class Shaders;

class CustomGLPolyDataMapper :public vtkOpenGLPolyDataMapper{
public:
    static CustomGLPolyDataMapper* New();
    std::shared_ptr<RenderDetail> renderDetail;
    vtkOpenGLBufferObject * tangentsBuffer;
    //vtkOpenGLVertexBufferObject* tangentsBuffer;
    std::vector<core::Vec3d>tangents;
    vtkTypeMacro(CustomGLPolyDataMapper, vtkOpenGLPolyDataMapper)
      virtual void initDraw();
    virtual void modified() {}
    virtual void draw() const { }
    virtual void SetMapperShaderParameters(vtkOpenGLHelper &cellBO, vtkRenderer *ren, vtkActor *act) override;
    void BuildBufferObjects(vtkRenderer *ren, vtkActor *act);
};

class VTKRenderDelegate : public RenderDelegate
{
public:
    virtual vtkActor *getActor() = 0;
    virtual void initDraw() override{}
    virtual void modified() override{}
    virtual void draw() const override{ }


    void setShadersProgram(vtkOpenGLPolyDataMapper *mapper,const std::string  &shaderPrograms);

    template<typename ShaderProgramType>
    void setShadersProgramReplacements(vtkOpenGLPolyDataMapper *mapper,
                                       const ShaderProgramType &shaderPrograms);
};




template<typename ShaderProgramType>
void VTKRenderDelegate::setShadersProgramReplacements(vtkOpenGLPolyDataMapper *mapper,
                                                      const ShaderProgramType &shaderPrograms)
{
    for(auto &shader : shaderPrograms)
    {
        for(auto &program : shader.second)
        {
            mapper->AddShaderReplacement(static_cast<vtkShader::Type>(shader.first),
                                         program[0],
                                         true,
                                         program[1],
                                         false);

        }
    }
}

#endif // SMRENDERABLE_H
