#ifndef VTKRENDERABLE_H
#define VTKRENDERABLE_H

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
