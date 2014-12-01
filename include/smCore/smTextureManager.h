/*
****************************************************
				SimMedTK LICENSE
****************************************************

****************************************************
*/

#ifndef SMTEXTUREMANAGER_H
#define SMTEXTUREMANAGER_H

#include "smCore/smConfig.h"
#include "smRendering/smConfigRendering.h"
#include "smCore/smErrorLog.h"

#include <GL/glew.h>
#include <GL/glut.h>
#include <IL/ilu.h>
#include <IL/ilut.h>
#include <QHash>


enum smTextureReturnType{
	SOFMIS_TEXTURE_NOTFOUND,
	SOFMIS_TEXTURE_IMAGELOADINGERROR,
	SOFMIS_TEXTURE_DRIVERNOTINITIALIZED,
	SOFMIS_TEXTURE_OK
};

enum ImageColorType{
	SOFMIS_IMAGECOLOR_RGB,
	SOFMIS_IMAGECOLOR_RGBA,
	SOFMIS_IMAGECOLOR_OFFSCREENRGBA,
	SOFMIS_IMAGECOLOR_DEPTH
};

struct smTexture{

	smChar textureFileName[SOFMIS_MAX_FILENAME_LENGTH];
	GLuint textureGLId;
	smInt width;
	smInt height;
	smInt bitsPerPixel;
	ImageColorType imageColorType;
	ILuint imageId;
	smGLUInt GLtype;
	smBool isInitialized;
	unsigned char *mRGB;
	smBool isTextureDataAvailable;

	inline smTexture *copy(){
		smTexture *myCopy=new smTexture();
		memcpy(myCopy->textureFileName,this->textureFileName,SOFMIS_MAX_FILENAME_LENGTH);
		myCopy->textureGLId=this->textureGLId;
		myCopy->width=this->width;
		myCopy->height=this->height;
		myCopy->bitsPerPixel=this->bitsPerPixel;
		myCopy->imageColorType=this->imageColorType;
		myCopy->imageId=this->imageId;
		myCopy->isInitialized=this->isInitialized;
		myCopy->isTextureDataAvailable=this->isTextureDataAvailable;

		if(this->isTextureDataAvailable){
			if(this->imageColorType==SOFMIS_IMAGECOLOR_RGBA){
				myCopy->mRGB=new unsigned char[4*this->width*this->height];
				memcpy(myCopy->mRGB,this->mRGB,4*this->width*this->height);
			}
			else{
				myCopy->mRGB=new unsigned char[3*this->width*this->height];
				memcpy(myCopy->mRGB,this->mRGB,3*this->width*this->height);
			}
		}
		return myCopy;
	}
};

struct smImageData{
	void *data;
	smInt width;
	smInt height;
	smInt bytePerPixel;
	ImageColorType imageColorType;
	smChar fileName[SOFMIS_MAX_FILENAME_LENGTH];
};

typedef void (*smCallTextureCallBack)(smImageData *imageData,void *);

class smTextureManager:public smCoreClass{

	static smErrorLog *errorLog;
	static vector<smTexture*> textures;
	static QHash<QString,smInt>textureIndexId;
	static smInt activeTextures;
	static smBool isInitialized;
	static smBool isInitializedGL;
	static smBool isDeleteImagesEnabled;
	static smCallTextureCallBack callback;
	static void *param;

	static void reportError(){
		ILenum error;
		while ((error = ilGetError()) != IL_NO_ERROR) {
			errorLog->addError((smChar *)iluErrorString(error));
		}
	}

public:
	static smTextureReturnType initGLTextures();

	static void init(smErrorLog * p_errorLog){
		errorLog=p_errorLog;
		ilInit();
		iluInit();
		ilutRenderer(ILUT_OPENGL);
		ilutEnable(ILUT_OPENGL_CONV);
		textures.resize(SOFMIS_MAX_TEXTURENBR);
		textures.clear();
		activeTextures=0;
		isDeleteImagesEnabled=true;
		isInitialized=true;
		callback=NULL;
	}

	static smTextureReturnType loadTexture(const smChar *p_fileName,
                                           const smChar *p_textureReferenceName,
                                           smInt &p_textureId);

	static smTextureReturnType loadTexture(const smChar *p_fileName,
                                           const smChar *p_textureReferenceName,
                                           smBool p_flipImage=false,
                                           bool deleteDataAfterLoaded=true);

	static smTextureReturnType loadTexture(const smString p_fileName,
                                           const smChar *p_textureReferenceName,
                                           smBool p_flipImage=false);

	static smTextureReturnType findTextureId(const smChar *p_textureReferenceName,
                                                                smInt &p_textureId);

	static GLuint activateTexture(smTexture *p_texture);
	static GLuint activateTexture(const smChar *p_textureReferenceName );
	static GLuint activateTexture(smInt p_textureId);

	static GLuint activateTexture(const smChar *p_textureReferenceName,
                                                        smInt p_textureGLOrder);

	static GLuint activateTexture(smTexture *p_texture, smInt p_textureGLOrder,
                                                        smInt p_shaderBindGLId);

	static GLuint activateTexture(const smChar *p_textureReferenceName,
                                  smInt p_textureGLOrder, smInt p_shaderBindName);

	static GLuint activateTexture(smInt p_textureId,smInt p_textureGLOrder);
	static void  activateTextureGL(GLuint  p_textureId, smInt p_textureGLOrder);
	static GLuint disableTexture(const smChar *p_textureReferenceName);

	static GLuint disableTexture(const smChar *p_textureReferenceName,
                                                        smInt p_textureGLOrder);

	static GLuint disableTexture(smInt p_textureId);
	static GLuint getOpenglTextureId(const smChar *p_textureReferenceName);
	static GLuint getOpenglTextureId(smInt p_textureId);

	static void registerGLLoadingFunc(smCallTextureCallBack p_test, void *p_param=NULL){
		callback=p_test;
		param=p_param;
	}

	static smTexture * getTexture(const smChar* p_textureReferenceName );
	static void createDepthTexture(const smChar *p_textureReferenceName,
                                                  smInt p_width,smInt p_height);

	static void  createColorTexture(const smChar *p_textureReferenceName,
                                                  smInt p_width,smInt p_height);

	static void initDepthTexture(smTexture *p_texture);
	static void initColorTexture(smTexture *p_texture);
	static void generateMipMaps(smInt p_textureId);
	static void generateMipMaps(const smChar *p_textureReferenceName );

	static void duplicateTexture(const smChar *p_textureReferenceName,
                                    smTexture *p_texture,ImageColorType p_type);

	static void copyTexture(const smChar *p_textureDestinationName,
                                             const smChar *p_textureSourceName);

	static void saveBinaryImage(smChar *p_binaryData,smInt p_width,
                                       smInt p_height,const smChar *p_fileName);

	static void saveRGBImage(smChar *p_binaryData,smInt p_width,
                                             smInt p_height,smChar *p_fileName);
};

#endif
