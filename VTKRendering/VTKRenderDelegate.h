#ifndef VTKRENDERABLE_H
#define VTKRENDERABLE_H

#include <vtkOpenGLPolyDataMapper.h>
#include "Core/RenderDelegate.h"

class vtkActor;
class CustomGLPolyDataMapper :public vtkOpenGLPolyDataMapper{
public:
	static CustomGLPolyDataMapper* New();
	vtkOpenGLBufferObject * tangentsBuffer;
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

    template<typename ShaderProgramType>
    void setShadersProgram(vtkOpenGLPolyDataMapper *mapper,
                           const ShaderProgramType &shaderPrograms);

    template<typename ShaderProgramType>
    void setShadersProgramReplacements(vtkOpenGLPolyDataMapper *mapper,
                                       const ShaderProgramType &shaderPrograms);
};

template<typename ShaderProgramType>
void VTKRenderDelegate::setShadersProgram(vtkOpenGLPolyDataMapper *mapper,
                                          const ShaderProgramType &shaderPrograms)
{
    for(const auto &program : shaderPrograms)
    {
        switch(static_cast<vtkShader::Type>(program.first))
        {
            case vtkShader::Fragment:
            {
                mapper->SetFragmentShaderCode(program.second.c_str());
				
                break;
            }
            case vtkShader::Vertex:
            {
                mapper->SetVertexShaderCode(program.second.c_str());
                break;
            }
            case vtkShader::Geometry:
            {
                mapper->SetGeometryShaderCode(program.second.c_str());
                break;
            }
            default:
            {
                std::cerr << "Unknown shader program." << std::endl;
            }
        }
    }
}

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
