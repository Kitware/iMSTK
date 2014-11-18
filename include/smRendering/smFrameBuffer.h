/*
****************************************************
                  SOFMIS LICENSE

****************************************************

    \author:    <http:\\acor.rpi.edu>
                SOFMIS TEAM IN ALPHABATIC ORDER
                Anderson Maciel, Ph.D.
                Ganesh Sankaranarayanan, Ph.D.
                Sreekanth A Venkata
                Suvranu De, Ph.D.
                Tansel Halic
                Zhonghua Lu

    \author:    Module by Tansel Halic
                
                
    \version    1.0
    \date       04/2009
    \bug	    None yet
    \brief	    This is framebuffer object for offline rendering or rendering to a texture

*****************************************************
*/
#ifndef SMFRAMEBUFFEROBJECT_H
#define SMFRAMEBUFFEROBJECT_H

#include <string.h>
#include <GL/glew.h>
#include <QVector>

#include "smCore/smConfig.h"
#include "smCore/smCoreClass.h"
#include "smCore/smErrorLog.h"
#include "smExternal/framebufferObject.h"
#include "smExternal/renderbuffer.h"
#include "smCore/smTextureManager.h"


enum smFBOImageAttachmentType{
	SOFMIS_FBOIMAGE_COLOR,
	SOFMIS_FBOIMAGE_DEPTH,
	SOFMIS_FBOIMAGE_STENCIL

};

enum smRenderBufferType{
	SOFMIS_RENDERBUFFER_DEPTH=GL_DEPTH_COMPONENT,
	SOFMIS_RENDERBUFFER_STENCIL=GL_STENCIL_INDEX,
	SOFMIS_RENDERBUFFER_COLOR_RGBA=GL_RGBA,
	SOFMIS_RENDERBUFFER_COLOR_RGB=GL_RGB



};

class smRenderBuffer:public smCoreClass{
protected:
	Renderbuffer _rb;
	smInt width;
	smInt height;
	smBool isAllocated;
	smRenderBufferType type;
	smInt attachmentOrder;
	
public:
	inline smInt getHeight(){return height;}
	inline smInt getWidth(){return width;}
	void setAttachmentOrder(smInt p_attachmentOrder){
		attachmentOrder=p_attachmentOrder;
	
	}
	smInt getAttachmentOrder(smInt p_attachmentOrder){
		return attachmentOrder;
	
	}
	GLenum getGLAttachmentId(){
		if(type==SOFMIS_RENDERBUFFER_DEPTH)
			return GL_DEPTH_ATTACHMENT_EXT;
		if(type==SOFMIS_RENDERBUFFER_STENCIL)
			return GL_STENCIL_ATTACHMENT;
		if(type==SOFMIS_RENDERBUFFER_COLOR_RGBA||type==SOFMIS_RENDERBUFFER_COLOR_RGB)
			return GL_COLOR_ATTACHMENT0_EXT+attachmentOrder;

		
	
	}
	inline smRenderBufferType getRenderBufType(){
		return type;
	}
	inline smGLUInt  getRenderBufId(){
		return _rb.GetId();
	}
	smRenderBuffer(){
		isAllocated=false;

	}

	smRenderBuffer(smRenderBufferType p_type,smInt p_width,smInt p_height){
		width=p_width;
		height=p_height;
		_rb.Set(p_type,width,height);
		isAllocated=true;
		type=p_type;

	
	}
	smBool createDepthBuffer(smInt width,smInt height){
		if(!isAllocated){
			_rb.Set(GL_DEPTH_COMPONENT,width,height);
			return true;
		}
		return false;
	
	}
	smBool createColorBuffer(){
		if(!isAllocated){
			_rb.Set(SOFMIS_RENDERBUFFER_COLOR_RGBA,width,height);
		}

		return false;
	}
	smBool createStencilBuffer(){
		if(!isAllocated){
			_rb.Set(SOFMIS_RENDERBUFFER_STENCIL,width,height);
		}
		return false;
	}

};



class smFrameBuffer:public smCoreClass{
public:	
	FramebufferObject _fbo;
	
	smBool isColorBufAttached;
	smBool isDepthTexAttached;
	
	//smTexture texture;
	smInt defaultColorAttachment;
	smRenderBuffer *renderBuffer;
	
public:
	smBool renderDepthBuff;
	smBool renderColorBuff;
	smInt width;
	smInt height;

	smFrameBuffer(){
		width=0;
		height=0;
		isColorBufAttached=false;
		isDepthTexAttached=false;
		renderDepthBuff=false;
		renderColorBuff=false;
		renderBuffer=NULL;
	}
	void setDim(smInt p_width,smInt p_height){
		width=p_width;
		height=p_height;
	}
	inline smInt getHeight(){return height;}
	inline smInt getWidth(){return width;}
	void attachTexture();
	void attachRenderBuffer(smRenderBuffer *p_renderBuf){
		if(p_renderBuf->getWidth()!=width||p_renderBuf->getHeight()!=height)
			_fbo.AttachRenderBuffer(p_renderBuf->getRenderBufId(),p_renderBuf->getGLAttachmentId());
	
	}
	void attachDepthTexture(smTexture *p_texture){
		/*if(renderBuffer==NULL){
			renderBuffer=new smRenderBuffer(SOFMIS_RENDERBUFFER_DEPTH,width,height);
			

		}*/
		if(p_texture==NULL)
		{
		  cout<<"Error in frambuffer depth attachment"<<endl;
		}

		_fbo.AttachTexture(p_texture->GLtype,p_texture->textureGLId,GL_DEPTH_ATTACHMENT_EXT);
		isDepthTexAttached=true;
	
	}
	/*void attachStencilTexture(smTexture *p_texture){
		_fbo.AttachTexture(p_texture->GLtype,p_texture->textureGLId,GL_STENCIL_INDEX);	
	}*/
	void attachColorTexture(smTexture *p_texture,smInt p_attachmentOrder){
		 defaultColorAttachment=p_attachmentOrder;
		_fbo.AttachTexture(p_texture->GLtype,p_texture->textureGLId,GL_COLOR_ATTACHMENT0_EXT+p_attachmentOrder);
		isColorBufAttached=true;
	
	}
	inline void activeColorBuf(smInt  p_order){
		glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT+p_order);
	
	}
	inline void enable(){
		_fbo.Bind();
		if(!isColorBufAttached){
		 glDrawBuffer(GL_NONE);
		 glReadBuffer(GL_NONE);
		}
	}
	inline void disable(){
		_fbo.Disable();
		
	}

	smBool checkStatus();
	void draw(smDrawParam p_params);
};

#endif
