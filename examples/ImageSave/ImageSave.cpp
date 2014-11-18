#include "ImageSave.h"

class ImageExample:public smCoreClass{

	smSDK* sofmisSDK;
public:	

	
	ImageExample(){
		///create the SDK
		sofmisSDK=smSDK::createSDK();


		///Init the texture manager before using it
		smTextureManager::init(smSDK::getErrorLog());

		///Generate a binary image
		smChar data[128*128];
		smChar data1[255*255*3];
		for(int j=0;j<128;j++)
			for(int i=0;i<128;i++)
			{
				data[i+j*128]=i%2*255;
				data[i+j*128]=j%2*255;
			}
			//save image
		for(int j=0;j<255;j++)
			for(int i=0;i<255;i++)
			{
				data1[i*3+j*255*3]=255;
				data1[i*3+j*255*3+1]=(i%2==0?255:120);
				data1[i*3+j*255*3+2]=255;
		  
		  }

		///save the data in JPG format. file extension(jpg) specifies the file format
		smTextureManager::saveRGBImage(data1,255,255,"ImageExampletest.jpg");
		cout<<"RGB Image is created"<<endl;
		///save the file in binary format. the extension will be always bmp.
		smTextureManager::saveBinaryImage(data,128,128,"ImageExampletest.bmp");
		cout<<"Bitmap Image is created"<<endl;
		///run the sdk
		sofmisSDK->run();

	}




};
void main(int argc, char** argv){
	ImageExample *img=new ImageExample();
	delete img;
	return;

}
