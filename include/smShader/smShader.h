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
    \brief	    This Module is for shader class. It basically based on OPENGL Shaders. But it can be applicable to any

*****************************************************
*/


#ifndef SMSHADER_H
#define SMSHADER_H


#include <string.h>
#include <GL/glew.h>
#include <QVector>
#include "smCore/smConfig.h"
#include "smCore/smCoreClass.h"
#include "smCore/smErrorLog.h"
#include <QMultiHash>
#include "smUtilities/smGLUtils.h"
#include "smUtilities/smMatrix44.h"


class smMesh;
class smSurfaceMesh;

struct smTextureShaderAssignment{
   smGLInt textureShaderGLassignment;///the id that smShader creates...
   smInt textureId;///Id from texture manager
   QString shaderParamName;///The parameters that shaders use
};


///this is the generic shader class. It provides loading, initializing, binding, 
///enabling disabling current shader functionality.Also it provides frequent check of the shader code 
///to make shader development easy. 
class smShader:public smCoreClass{
 public :
		smGLInt tangentAttrib;
 protected:
    
    static QHash<smInt,smShader *>shaders;
	
    smChar vertexProgFileName[SOFMIS_MAX_FILENAME_LENGTH];
    smChar fragmentProgFileName[SOFMIS_MAX_FILENAME_LENGTH];
    smChar geometryProgFileName[SOFMIS_MAX_FILENAME_LENGTH];
    ///Error Loging
    smErrorLog *log;
    ///stores the content of the vertex shader file
    smChar *vertexShaderContent;
    ///stores the content of the fragment  shader file
    smChar *fragmentShaderContent;
    ///stores the content of the geometry shader file
    smChar *geometryShaderContent;
    ///if the vertex shader exists this will be true
    smBool vertexProgramExist;
    ///if the fragment shader exists this will be true
    smBool fragmentProgramExist;
    ///if the geometry shader exists this will be true
    smBool geometryProgramExist;
    
    ///stores the parameters for vertex shader
    QVector <smChar*>vertexShaderParamsString;
    ///stores the parameters for fragment shader
    QVector <smChar*>fragmentShaderParamsString;
    ///stores the parameters for geometry shader
    QVector <smChar*>geometryShaderParamsString;

    ///stores the attribute parameters
    QVector <smChar*>attribParamsString;
    ///error text for querying the opengl errors mostly
    smChar errorText[SOFMIS_MAX_ERRORLOG_TEXT];

    ///time for periodically checnking the shader
    QTime time;
	QMultiHash<smInt,smTextureShaderAssignment> texAssignments;
	void getAttribAndParamLocations();
	
	smChar modelViewMatrixName[SOFMIS_MAX_SHADERVARIABLENAME];
	smChar projectionMatrixName[SOFMIS_MAX_SHADERVARIABLENAME];
    
    
#ifdef SOFMIS_OPENGL_SHADER
    ///vertex shader object
    GLhandleARB     vertexShaderObject;
    ///fragment  shader object
    GLhandleARB   fragmentShaderObject;
    ///fragment  shader object
    GLhandleARB   geometryShaderObject;
    ///shader program object. 
    GLhandleARB    shaderProgramObject;
    ///stores the id of the parameters in vertex shader
    QVector <GLint>vertexShaderParams;
    ///stores the id of the parameters in fragment shader
    QVector <GLint>fragmentShaderParams;
    ///stores the id of the parameters in geometry shader
    QVector <GLint>geometryShaderParams;

	///stores the id of the parameters in geometry shader
    QVector <GLint>attribShaderParams;
    
    ///creates GLSL vertex shader
    void createVertexShaderGLSL();
    ///creates GLSL fragment shader
    void createFragmentShaderGLSL();
    ///creates GLSL geometry shader
    void createGeometryShaderGLSL();

    ///reloads the vertex shader
    void reloadVertexShaderGLSL();
    ///reloads the fragment shader
    void reloadFragmentShaderGLSL();
    ///reloads the geometry shader
    void reloadGeometryShaderGLSL();

    GLint addVertexShaderParamGLSL(smChar* p_paramVertex);
    GLint addFragmentShaderParamGLSL(smChar* p_paramFragment);
    GLint addGeometryShaderParamGLSL(smChar* p_paramGeometry);

public:
	GLuint getProgramObject(){return shaderProgramObject;} 
	GLuint getVertexShaderObject(){return vertexShaderObject;} 
	GLuint getFragmentShaderObject(){return fragmentShaderObject;} 
	GLuint getGeometryShaderObject(){return geometryShaderObject;} 
	
	///Attaches The texture ID to the mesh 
	void attachTexture(smUnifiedID p_meshID,smInt p_textureID);

	///assigns the texture by name if you don't know the textureID
	smBool attachTexture(smUnifiedID p_meshID,smChar * const p_textureName, smChar * const p_textureShaderName);

protected:
	///This stores the opengl binded texture id
	QHash<QString,smGLInt>textureGLBind;

	void autoGetTextureIds();
    
#endif
    ///reloads all shaders
    smBool reLoadAllShaders();
    ///check opengl error
    smBool checkGLError();



 public:

    ///if the error check is enabled or not. If it is checked, opengl errors are queried and if there is, they will be stored in logger
    smBool checkErrorEnabled;
   
     ///constructor gets the error log class
    smShader(smErrorLog *log=NULL);
    
    ///initialized the shaders.
    /// \param vertexProgFileName   vertex program file name
    /// \param fragmentProgFileName fragment program file name
    /// \param geometryProgFileName geometry program file name
    smBool initShaders(smChar *vertexProgFileName,smChar *fragmentProgFileName,smChar *geometryProgFileName);
    
    /// enables the shader
    void  enableShader();

    /// disables the shader
    void disableShader();

	///Initialize the shader..This is called automatically.
	virtual void initDraw(smDrawParam p_param);

#ifdef SOFMIS_OPENGL_SHADER
    ///add parameter for Vertex Shader
    GLint addVertexShaderParam(smChar* p_paramVertex);
    ///add parameter for Fragment Shader
    GLint addFragmentShaderParam(smChar* p_paramFragment);
    ///add parameter for Geometry Shader
    GLint addGeometryShaderParam(smChar* p_paramGeometry);
    ///add parameter for all Shaders
    GLint addShaderParamForAll(smChar* p_paramName);
    ///attrib parameters for Shaders
    GLint addShaderParamAttrib(smChar* p_paramName);

	void createTextureParam(smChar *p_textureNameInShaderCode);

	smBool setShaderFileName(smChar *p_vertexFileName,smChar *p_geometryFileName, smChar *p_fragmentFileName);
	smBool setModelViewMatrixShaderName(smChar *p_modelviewMatrixName){
		if(strlen(p_modelviewMatrixName)>SOFMIS_MAX_SHADERVARIABLENAME-1){
		 return false;
		}
		else 
			strcpy(this->modelViewMatrixName,p_modelviewMatrixName);

		createParam(modelViewMatrixName);
		return true;
	
	}
	smBool setProjectionMatrixShaderName(smChar *p_projectionName){
		if(strlen(p_projectionName)>SOFMIS_MAX_SHADERVARIABLENAME-1){
		 return false;
		}
		else 
			strcpy(this->projectionMatrixName,p_projectionName);
		createParam(projectionMatrixName);
		return true;
	
	}
	///holds the unitform location for projection matrix. That is needed in newer versions of GLSL 
	smGLInt projectionMatrix;
	
	///holds the unitform location for modelview matrix. That is needed in newer versions of GLSL 
	smGLInt modelViewMatrix;
	inline void updateGLSLMatwithOPENGL(){
		smMatrix44f proj,model;
		smGLUtils::queryModelViewMatrix<float>(model);
		smGLUtils::queryProjectionMatrix<float>(proj);

		//as the our matrix is row major, we need transpose it. Transpose parameters are true
		glUniformMatrix4fv(modelViewMatrix,1,true,(GLfloat*)model.e);
		glUniformMatrix4fv(projectionMatrix,1,true,(GLfloat*)proj.e);
	
	
	
	}

	///returns the shader attrrib param
	smGLInt getShaderParamForAll(smChar *p_paramName);

	///returns the fragment shader uniform param
	smGLInt getFragmentShaderParam(smChar *p_paramName);

	///returns the attrib location param
	smGLInt getShaderAtrribParam(smChar *p_paramName);

	void createParam( smChar * const p_param);
	smInt createAttrib( smChar * const p_attrib);

	smGLInt queryUniformLocation(smChar *const p_param){
		return glGetUniformLocation(shaderProgramObject, p_param);
	}


#endif

    

    ///checks whether the shader routine is updated
    smBool checkShaderUpdate(smInt milliseconds);
    
    ///turn on/off the error checking
    void enableCheckingErrors(smBool p_checkError);

    ///cleans up of the shader objects
    ~smShader(){
        #ifdef SOFMIS_OPENGL_SHADER
          if(vertexProgramExist)
            glDeleteObjectARB(vertexShaderObject);
		if(fragmentProgramExist)
			glDeleteObjectARB(fragmentShaderObject);
		if(geometryProgramExist)
            glDeleteObjectARB(geometryShaderObject);
		
        #endif
        
    
    }


	virtual void predraw(smMesh *mesh){
	}
	virtual void predraw(smSurfaceMesh *mesh){
	}
	virtual void posdraw(smMesh *mesh){
	}
	virtual void posdraw(smSurfaceMesh *mesh){
	}
	static inline smShader * getShader(smUnifiedID p_shaderID){
		return shaders[p_shaderID.ID];
	}
	static void initGLShaders(smDrawParam p_param);
	void activeGLTextures(smUnifiedID p_id);
	void activeGLVertAttribs(smInt p_id, smVec3f *p_vecs,smInt p_size);
	void registerShader();
	void print();

	///stores the current Active shader. 
	static smShader *currentShader;

	//It is also used to save and restore the current shader is disabled for a while to use 
	///default opengl rendering. The function saveAndDisableCurrent/restoreAndEnable perform save and restore the functions. 
	static smShader * savedShader;
	///if the currentShader is enabled or not
	static smBool currentShaderEnabled;
	///restores the shader from the last saved one
	static void restoreAndEnableCurrent();
	///save the last active shader and disable it.
	static void saveAndDisableCurrent();
};





#endif


