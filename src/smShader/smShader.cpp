#include "smShader/smShader.h"
#include "smUtilities/smGLUtils.h"
#include <fstream>
#include <iostream>
#include <GL/glew.h>
#include <GL/glut.h>
using namespace std;

QHash<smInt, smShader *> smShader::shaders;
smShader *smShader ::currentShader=NULL;
smShader *smShader ::savedShader=NULL;
smBool smShader::currentShaderEnabled=false;

void printInfoLog(GLhandleARB obj){

	int infologLength = 0;
	int charsWritten = 0;
	char *infoLog;

	glGetObjectParameterivARB(obj, GL_OBJECT_INFO_LOG_LENGTH_ARB, &infologLength);

	if (infologLength > 0){
		infoLog = (char *)malloc(infologLength);
		glGetInfoLogARB(obj, infologLength, &charsWritten, infoLog);
		printf("%s\n",infoLog);
		free(infoLog);
	}
}

smShader::smShader(smErrorLog *log){

	type=SIMMEDTK_SMSHADER;
	memset(vertexProgFileName,'\0',SIMMEDTK_MAX_FILENAME_LENGTH);
	memset(fragmentProgFileName,'\0',SIMMEDTK_MAX_FILENAME_LENGTH);
	memset(geometryProgFileName,'\0',SIMMEDTK_MAX_FILENAME_LENGTH);
	this->log=log;
	checkErrorEnabled=false;
	time.start();
	setModelViewMatrixShaderName("ModelMatrix");
	setProjectionMatrixShaderName("ProjectionMatrix");
}

///this function gets the vertex,fragment and geometry shader fileNames respectively. if you don't use
/// one of them just simply send NULL pointer as a parameter.
smBool smShader::initShaders(smChar *p_vertexProgFileName,smChar *p_fragmentProgFileName,smChar *p_geometryProgFileName){

	ifstream vertexShaderFile;
	ifstream fragmentShaderFile;
	ifstream geometryShaderFile;
	smLongInt fileSize;

	if(glewIsSupported("GL_VERSION_2_0")==GL_FALSE){
		if(log!=NULL)
			log->addError(this,"smShader:OpenGL 2.0 not supported");
		return false;
	}

	shaderProgramObject=glCreateProgram();
	if(p_vertexProgFileName!=NULL&&strlen(p_vertexProgFileName)>0){
		vertexShaderFile.open(p_vertexProgFileName);
		if(vertexShaderFile){
			strcpy(this->vertexProgFileName,p_vertexProgFileName);
			vertexShaderFile.seekg (0, ios::end);
			fileSize=vertexShaderFile.tellg();
			vertexShaderFile.seekg (0, ios::beg);
			vertexShaderContent = new smChar [fileSize+1];
			memset(vertexShaderContent,'\0',fileSize);
			vertexShaderFile.read(vertexShaderContent,fileSize); 
			cout<<"[initShaders] "<<p_vertexProgFileName;
		}
		else {
			if(log!=NULL)
				log->addError(this,"smShader:Vertex shader file couldn't be opened");
			return false;
		}

		vertexShaderFile.close();

		#ifdef SIMMEDTK_OPENGL_SHADER
			createVertexShaderGLSL();
			delete vertexShaderContent;
			//check the opengl error.
			if(checkErrorEnabled)
				checkGLError();
			vertexProgramExist=true;
		#endif
	}
	else{
		vertexShaderObject=NULL;
		vertexProgramExist=false;
	}

	if(p_fragmentProgFileName!=NULL&&strlen(p_fragmentProgFileName)>0){
		fragmentShaderFile.open(p_fragmentProgFileName);
		if(fragmentShaderFile){
			strcpy(this->fragmentProgFileName,p_fragmentProgFileName);
			fragmentShaderFile.seekg (0, ios::end);
			fileSize=fragmentShaderFile.tellg();
			fragmentShaderFile.seekg (0, ios::beg);
			fragmentShaderContent = new smChar [fileSize];
			memset(fragmentShaderContent,'\0',fileSize);
			fragmentShaderFile.read(fragmentShaderContent,fileSize);
			cout<<"[initShaders] "<<p_fragmentProgFileName<<endl;
		}
		else {
			if(log!=NULL)
				log->addError(this,"smShader:Fragment shader file couldn't be opened");
			return false;
		}
		fragmentShaderFile.close();
		#ifdef SIMMEDTK_OPENGL_SHADER
			createFragmentShaderGLSL();
			delete fragmentShaderContent;
			//check the opengl error.
			if(checkErrorEnabled)
				checkGLError();
			fragmentProgramExist=true;
		#endif
	}
	else{
		fragmentShaderObject=NULL;
		fragmentProgramExist=false;
	}

	if(p_geometryProgFileName!=NULL&&strlen(p_geometryProgFileName)>0){
		geometryShaderFile.open(p_geometryProgFileName);
		if(geometryShaderFile){
			strcpy(this->geometryProgFileName,p_geometryProgFileName);
			geometryShaderFile.seekg (0, ios::end);
			fileSize=geometryShaderFile.tellg();
			geometryShaderFile.seekg (0, ios::beg);
			geometryShaderContent = new smChar [fileSize];
			memset(geometryShaderContent,'\0',fileSize);
			geometryShaderFile.read(geometryShaderContent,fileSize);
		}
		else {
			if(log!=NULL)
				log->addError(this,"smShader:Fragment shader file couldn't be opened");
			return false;
		}
		geometryShaderFile.close();
		#ifdef SIMMEDTK_OPENGL_SHADER
			createGeometryShaderGLSL();
			delete geometryShaderContent;
			//check the opengl error.
			if(checkErrorEnabled)
				checkGLError();

			geometryProgramExist=true;
			//note that:based on geometry shader needw, the input and output parameters for geometry shader below may change
			glProgramParameteriEXT(shaderProgramObject, GL_GEOMETRY_INPUT_TYPE_EXT, GL_TRIANGLES);
			glProgramParameteriEXT(shaderProgramObject, GL_GEOMETRY_OUTPUT_TYPE_EXT, GL_TRIANGLE_STRIP);
			glProgramParameteriEXT(shaderProgramObject, GL_GEOMETRY_VERTICES_OUT_EXT, 1024);
		#endif
	}
	else{
		geometryShaderObject=NULL;
		geometryProgramExist=false;
	}

	glLinkProgram(shaderProgramObject);
	printInfoLog(shaderProgramObject);
	modelViewMatrix=glGetUniformLocation(shaderProgramObject,modelViewMatrixName);
	projectionMatrix=glGetUniformLocation(shaderProgramObject,projectionMatrixName);

	return true;
}

void smShader::createVertexShaderGLSL(){

	vertexShaderObject=glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShaderObject, 1, (const smChar**)&vertexShaderContent, NULL);
	glCompileShader(vertexShaderObject);
	printInfoLog(vertexShaderObject);
	if(checkErrorEnabled)
		checkGLError();

	glAttachShader(shaderProgramObject,vertexShaderObject);

	if(checkErrorEnabled)
		checkGLError();
}

void smShader::createFragmentShaderGLSL(){

	fragmentShaderObject=glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShaderObject, 1, (const smChar**)&fragmentShaderContent, NULL);
	glCompileShader(fragmentShaderObject);
	printInfoLog(fragmentShaderObject);
	if(checkErrorEnabled)
		checkGLError();

	glAttachShader(shaderProgramObject,fragmentShaderObject);

	if(checkErrorEnabled)
		checkGLError();
}
void smShader::createGeometryShaderGLSL(){

	geometryShaderObject=glCreateShader(GL_GEOMETRY_SHADER_EXT);
	if(checkErrorEnabled)
		checkGLError();
	glShaderSource(geometryShaderObject, 1, (const smChar**)&geometryShaderContent, NULL);
	glCompileShader(geometryShaderObject);
	if(checkErrorEnabled)
		checkGLError();
	glAttachShader(shaderProgramObject,geometryShaderObject);
	if(checkErrorEnabled)
		checkGLError();
}



void smShader::reloadVertexShaderGLSL(){

	glShaderSource(vertexShaderObject, 1, (const smChar**)&vertexShaderContent, NULL);
	glCompileShader(vertexShaderObject);
	if(checkErrorEnabled)
		checkGLError();
}

void smShader::reloadFragmentShaderGLSL(){

	glShaderSource(fragmentShaderObject, 1, (const smChar**)&fragmentShaderContent, NULL);
	glCompileShader(fragmentShaderObject);
	if(checkErrorEnabled)
		checkGLError();
}

///checks the opengl error 
smBool smShader::checkGLError(){

	memset(errorText,'\0',SIMMEDTK_MAX_ERRORLOG_TEXT);
	if(smGLUtils::queryGLError(errorText)){
		if(log!=NULL)
			log->addError(this,errorText);
		return false;
	}
	else
		return true;
}

void smShader::reloadGeometryShaderGLSL(){

	glShaderSource(geometryShaderObject, 1, (const smChar**)&geometryShaderContent, NULL);
	glCompileShader(geometryShaderObject);
	if(checkErrorEnabled)
		checkGLError();
}


///enable the shader
void smShader::enableShader(){

	#ifdef SIMMEDTK_OPENGL_SHADER
		if (vertexProgramExist)
			glEnable(GL_VERTEX_PROGRAM_ARB);
		if(fragmentProgramExist)
			glEnable(GL_FRAGMENT_PROGRAM_ARB);
		if(geometryProgramExist)
			glEnable(GL_GEOMETRY_SHADER_ARB);
		glUseProgramObjectARB(shaderProgramObject);
		smShader::currentShader=this;
		smShader::currentShaderEnabled=true;
	#endif
}

///disable the shader
void smShader::disableShader(){

	#ifdef SIMMEDTK_OPENGL_SHADER
		if(vertexProgramExist)
			glDisable(GL_VERTEX_PROGRAM_ARB);
		if(fragmentProgramExist)
			glDisable(GL_FRAGMENT_PROGRAM_ARB);
		if(geometryProgramExist)
			glDisable(GL_GEOMETRY_SHADER_ARB);
		glUseProgramObjectARB(0);
		smShader::currentShader=this;
		smShader::currentShaderEnabled=false;
	#endif
}

///enable the shader
void smShader::restoreAndEnableCurrent(){

	#ifdef SIMMEDTK_OPENGL_SHADER
		if(smShader::savedShader!=NULL){
			smShader::currentShader=smShader::savedShader;
			if (currentShader->vertexProgramExist)	glEnable(GL_VERTEX_PROGRAM_ARB);
			if(currentShader->fragmentProgramExist)	glEnable(GL_FRAGMENT_PROGRAM_ARB);
			if(currentShader->geometryProgramExist)	glEnable(GL_GEOMETRY_SHADER_ARB);
			glUseProgramObjectARB(currentShader->shaderProgramObject);
			smShader::currentShaderEnabled=true;
		}
	#endif
}

///disable the shader
void smShader::saveAndDisableCurrent(){

	#ifdef SIMMEDTK_OPENGL_SHADER
		if(currentShader!=NULL){
			if (smShader::currentShader->vertexProgramExist)
				glDisable(GL_VERTEX_PROGRAM_ARB);
			if(smShader::currentShader->fragmentProgramExist)
				glDisable(GL_FRAGMENT_PROGRAM_ARB);
			if(smShader::currentShader->geometryProgramExist)
				glDisable(GL_GEOMETRY_SHADER_ARB);
			smShader::currentShaderEnabled=false;
			smShader::savedShader=smShader::currentShader;
			glUseProgramObjectARB(0);
		}
	#endif
}

smGLInt smShader::addVertexShaderParamGLSL(smChar* p_paramNameVertex){

	smGLInt param;
	param=glGetUniformLocation(shaderProgramObject,p_paramNameVertex);
	vertexShaderParamsString.push_back(p_paramNameVertex);
	vertexShaderParams.push_back(param);

	return param;
}

smGLInt smShader::addFragmentShaderParamGLSL(smChar* p_paramNameFragment){

	smGLInt param;
	param=glGetUniformLocation(shaderProgramObject,p_paramNameFragment);
	if(checkErrorEnabled)
		checkGLError();
	fragmentShaderParamsString.push_back(p_paramNameFragment);
	fragmentShaderParams.push_back(param);
	return param;
}

smGLInt smShader::addGeometryShaderParamGLSL(smChar* p_paramNameGeometry){

	smGLInt param;
	param=glGetUniformLocation(shaderProgramObject,p_paramNameGeometry);
	geometryShaderParamsString.push_back(p_paramNameGeometry);
	geometryShaderParams.push_back(param);

	return param;
}

smGLInt smShader::addVertexShaderParam(smChar* p_paramNameVertex){

	#ifdef SIMMEDTK_OPENGL_SHADER
		return addVertexShaderParamGLSL(p_paramNameVertex);
	#endif
}
smGLInt smShader::addFragmentShaderParam(smChar* p_paramNameFragment){

	#ifdef SIMMEDTK_OPENGL_SHADER
		return addFragmentShaderParamGLSL(p_paramNameFragment);
	#endif
}

smGLInt smShader::addGeometryShaderParam(smChar* p_paramNameGeometry){

	#ifdef SIMMEDTK_OPENGL_SHADER
		return addGeometryShaderParamGLSL(p_paramNameGeometry);
	#endif
}

smGLInt smShader::addShaderParamForAll(smChar* p_paramName){

	#ifdef SIMMEDTK_OPENGL_SHADER
		smGLInt param;
		param=glGetUniformLocation(shaderProgramObject,p_paramName);
		vertexShaderParamsString.push_back(p_paramName);
		vertexShaderParams.push_back(param);

		fragmentShaderParamsString.push_back(p_paramName);
		fragmentShaderParams.push_back(param);

		geometryShaderParamsString.push_back(p_paramName);
		geometryShaderParams.push_back(param);

		textureGLBind.insert(p_paramName,param);
		return param;
	#endif
}

smGLInt smShader::getShaderParamForAll(smChar *p_paramName){

	#ifdef SIMMEDTK_OPENGL_SHADER
		QString p_param(p_paramName);
		for(smInt i=0;i<vertexShaderParamsString.size();i++){
			if(QString(vertexShaderParamsString[i])==p_param)
				return vertexShaderParams[i];
		}
		return -1;
	#endif
}

smGLInt smShader::getFragmentShaderParam(smChar *p_paramName){

	#ifdef SIMMEDTK_OPENGL_SHADER
		QString p_param(p_paramName);
		for(smInt i=0;i<fragmentShaderParamsString.size();i++){
			if(QString(fragmentShaderParamsString[i])==p_param)
				return fragmentShaderParams[i];
		}
		return -1;
	#endif
}

smGLInt smShader::getShaderAtrribParam(smChar *p_paramName){

	#ifdef SIMMEDTK_OPENGL_SHADER
		QString p_param(p_paramName);
		for(smInt i=0;i<attribParamsString.size();i++){
			if(QString(attribParamsString[i])==p_param)
				return attribShaderParams[i];
		}
		return -1;
	#endif
}
GLint smShader::addShaderParamAttrib(smChar* p_paramName){

	smGLInt param;
	param=glGetAttribLocationARB(shaderProgramObject,p_paramName);

	if(checkErrorEnabled)
		checkGLError();

	return param;
}

smBool smShader::reLoadAllShaders(){

	ifstream vertexShaderFile;
	ifstream fragmentShaderFile;
	ifstream geometryShaderFile;
	smLongInt fileSize;

	if(vertexProgramExist==true){
		vertexShaderFile.open(vertexProgFileName);
		if(vertexShaderFile){
			vertexShaderFile.seekg (0, ios::end);
			fileSize=vertexShaderFile.tellg();
			vertexShaderFile.seekg (0, ios::beg);
			vertexShaderContent = new smChar [fileSize];
			memset(vertexShaderContent,'\0',fileSize);
			vertexShaderFile.read(vertexShaderContent,fileSize);
		}
		else {
			if(log!=NULL)
				log->addError(this,"smShader:Vertex shader file couldn't be opened");
			return false;
		}
		vertexShaderFile.close();
		#ifdef SIMMEDTK_OPENGL_SHADER
		reloadVertexShaderGLSL();
		delete vertexShaderContent;
		//check the opengl error.
		if(checkErrorEnabled)
			if( smGLUtils::queryGLError(errorText))
				 if(log!=NULL)
					log->addError(this,errorText);
		#endif
	}
	else
		vertexShaderObject=NULL;

	if(fragmentProgramExist==true){
		fragmentShaderFile.open(fragmentProgFileName);
		if(fragmentShaderFile){
			fragmentShaderFile.seekg (0, ios::end);
			fileSize=fragmentShaderFile.tellg();
			fragmentShaderFile.seekg (0, ios::beg);
			fragmentShaderContent = new smChar [fileSize];
			memset(fragmentShaderContent,'\0',fileSize);
			fragmentShaderFile.read(fragmentShaderContent,fileSize); 
		}
		else {
			if(log!=NULL)
				log->addError(this,"smShader:Fragment shader file couldn't be opened");
			return false;
		}
		fragmentShaderFile.close();

		#ifdef SIMMEDTK_OPENGL_SHADER
			reloadFragmentShaderGLSL();
			delete fragmentShaderContent;
			//check the opengl error.
			if(checkErrorEnabled)
				if(smGLUtils::queryGLError(errorText))
					if(log!=NULL)
						log->addError(this,errorText);
		#endif
	}
	else
		fragmentShaderObject=NULL;


	if(geometryProgramExist==true){
		geometryShaderFile.open(geometryProgFileName);
		if(geometryShaderFile){
			geometryShaderFile.seekg (0, ios::end);
			fileSize=geometryShaderFile.tellg();
			geometryShaderFile.seekg (0, ios::beg);
			geometryShaderContent = new smChar [fileSize];
			memset(geometryShaderContent,'\0',fileSize);
			geometryShaderFile.read(geometryShaderContent,fileSize); 
		}
		else {
			if(log!=NULL)
				log->addError(this,"smShader:Fragment shader file couldn't be opened");
			return false;
		}

		geometryShaderFile.close();

		#ifdef SIMMEDTK_OPENGL_SHADER
			reloadGeometryShaderGLSL();
			delete geometryShaderContent;
			//check the opengl error.
			if(checkErrorEnabled)
				if(smGLUtils::queryGLError(errorText))
					if(log!=NULL)
						log->addError(this,errorText);
		#endif
	}
	else
		geometryShaderObject=NULL;

	glLinkProgram(shaderProgramObject);
	if(checkErrorEnabled)
		if(smGLUtils::queryGLError(errorText))
			if(log!=NULL)
				log->addError(this,errorText);

	return true;
}
///checks the shader source code within the given interval in milliseconds
smBool smShader::checkShaderUpdate(smInt interval){

	if (time.elapsed()>interval){
		time.start();
		return reLoadAllShaders();
	}
}

void smShader::enableCheckingErrors(smBool p_checkError){

	this->checkErrorEnabled=p_checkError;
}

void smShader::attachTexture(smUnifiedID p_meshID,smInt p_textureID){

	smTextureShaderAssignment assign;
	assign.textureId=p_textureID;
	texAssignments.insert(p_meshID.ID,assign);
}

smBool smShader::attachTexture(smUnifiedID p_meshID,
                               smChar * const p_textureName,
                               smChar * const p_textureShaderName){

	smTextureShaderAssignment assign;
	if(smTextureManager::findTextureId(p_textureName,assign.textureId)==SIMMEDTK_TEXTURE_NOTFOUND){
		cout<<"texture "<<p_textureName<<" is not found in shader:"<<p_textureShaderName<<"  for mesh id:"<< p_meshID.ID<<  endl;
		return false;
	}
	assign.shaderParamName=p_textureShaderName;
	texAssignments.insert(p_meshID.ID,assign);

	return true;
}

void smShader::autoGetTextureIds(){

	QMultiHash<smInt,smTextureShaderAssignment>::iterator i=texAssignments.begin() ;
	for(;i!=texAssignments.end();i++){
		i.value().textureShaderGLassignment=textureGLBind[i.value().shaderParamName];
	}
}

void smShader::createTextureParam(smChar *p_textureNameInShaderCode){

	this->textureGLBind[p_textureNameInShaderCode]=-1;
}

smBool smShader::setShaderFileName(smChar *p_vertexFileName,
                                   smChar *p_geometryFileName,
                                   smChar *p_fragmentFileName){

	if(p_vertexFileName!=NULL){
		if(strlen(p_vertexFileName)>SIMMEDTK_MAX_FILENAME_LENGTH){
			if(!log==NULL){
				log->addError("Vertex Shader File Name is very long");
				return false;
			}
		}
		memcpy(vertexProgFileName,p_vertexFileName,SIMMEDTK_MAX_FILENAME_LENGTH);
	}

	if(p_geometryFileName!=NULL){
		if(strlen(geometryProgFileName)>SIMMEDTK_MAX_FILENAME_LENGTH){
			if(!log==NULL){
				log->addError("VertexGeometry Shader File Name is very long");
				return false;
			}
		}
		memcpy(geometryProgFileName,p_geometryFileName,SIMMEDTK_MAX_FILENAME_LENGTH);
	}

	if(p_fragmentFileName!=NULL){
		if(strlen(fragmentProgFileName)>SIMMEDTK_MAX_FILENAME_LENGTH){
			if(!log==NULL){
				log->addError("Fragment Shader File Name is very long");
				return false;
			}
		}
		memcpy(fragmentProgFileName,p_fragmentFileName,SIMMEDTK_MAX_FILENAME_LENGTH);
	}
	return true;
}

void smShader::initDraw(smDrawParam p_param){

	initShaders(vertexProgFileName,fragmentProgFileName,geometryProgFileName);
	getAttribAndParamLocations();
	autoGetTextureIds();
}

smInt smShader::createAttrib( smChar * const p_attrib){

	attribParamsString.push_back(p_attrib);
	return attribParamsString.size();
}

void smShader::createParam( smChar *const p_param){

	vertexShaderParamsString.push_back(p_param);
	fragmentShaderParamsString.push_back(p_param);
	geometryShaderParamsString.push_back(p_param);
}

void smShader::getAttribAndParamLocations(){

	smGLInt param;
	for(smInt i=0;i<vertexShaderParamsString.size();i++){
		param=glGetUniformLocation(shaderProgramObject,vertexShaderParamsString[i]);
		vertexShaderParams.insert(i,param);
		if(textureGLBind[vertexShaderParamsString[i]]!=-1)
			textureGLBind[vertexShaderParamsString[i]]=param;
	}
	for(smInt i=0;i<fragmentShaderParamsString.size();i++){
		param=glGetUniformLocation(shaderProgramObject,fragmentShaderParamsString[i]);
		fragmentShaderParams.insert(i,param);
		cout<<"[smShader::getAttribAndParamLocations] "<<fragmentShaderParamsString[i]<<" "<<param <<endl;
		if(textureGLBind[fragmentShaderParamsString[i]]!=-1)
			textureGLBind[fragmentShaderParamsString[i]]=param;
	}
	for(smInt i=0;i<geometryShaderParamsString.size();i++){
		param=glGetUniformLocation(shaderProgramObject,geometryShaderParamsString[i]);
		geometryShaderParams.insert(i,param);
		if(textureGLBind[geometryShaderParamsString[i]]!=-1)
			textureGLBind[geometryShaderParamsString[i]]=param;
	}
	for(smInt i=0;i< attribParamsString.size();i++){
		param=glGetAttribLocation(shaderProgramObject,attribParamsString[i]);
		attribShaderParams.insert(i,param);
	}
}


void smShader::initGLShaders(smDrawParam p_param){
	foreach (smShader *shader, shaders)
		shader->initDraw(p_param);
}

void smShader::activeGLTextures(smUnifiedID p_id){
	smInt counter=0;
	QList<smTextureShaderAssignment> values=texAssignments.values(p_id.ID) ;
	for(smInt i=0;i<values.size();i++){
		smTextureManager::activateTexture(values[i].textureId,counter);
		glUniform1iARB(values[i].textureShaderGLassignment, counter);
		counter++;
	}
}

void smShader::activeGLVertAttribs(smInt p_id, smVec3f *p_vecs,smInt p_size){
	glVertexAttribPointer(attribShaderParams[p_id], 3, smGLFloatType, GL_FALSE, 0, p_vecs);
}
void smShader::registerShader(){
	shaders.insert(this->uniqueId.ID,this);
}

void smShader::print(){
	for(smInt i=0;i<vertexShaderParamsString.size();i++){
		cout<<"Param:"<<vertexShaderParamsString[i]<<endl;
	}
}
