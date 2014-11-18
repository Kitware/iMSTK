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

    \author:    Module by Ganesh Sankaranarayanan
                
                
    \version    1.0
    \date       12/2010
    \bug	    None yet
    \brief	    This Module is the ADU Interface from Ontrak Control Systems Inc.  for interaction in SoFMIS
    

*****************************************************

*/
#ifdef WIN32 //right now adutux(aduhid on linux) requires a different interface

#define __CPLUSPLUS

#include "smExternalDevices/smADUInterface.h"
#include <iostream>
#include <fstream>
#include "AduHid.h"
//#include <Windows.h>



using namespace std;

/// default constructor

void smADUInterface::init(){

}

void smADUInterface::exec(){

this->start();
//run();
}
smADUInterface::smADUInterface(){


	serialNumber = new smChar[10];
	isOpened = false;
	calibrationData = new ADUDeviceCalibrationData();

	deviceData = new ADUDeviceData();
	serialNumber = "B02363";

	calibrationData->minValue1=0;
	calibrationData->maxValue1=1;
	calibrationData->minValue2=0;
	calibrationData->maxValue2=1;

	if(openDevice(serialNumber)== SOFMIS_MSG_SUCCESS){
		cout<<"ADU USB Device Opened Successfully"<<endl;
	}
	else
		cout<<"Check the USB connection of the ADU device or the serial number: Couldn't initialize the device"<<endl;


	ADUpipe = new smPipe("ADU_Data",sizeof(ADUDeviceData),10);


	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
}


smADUInterface::smADUInterface(char *calibrationFile){

// Read device serial number and calibration values 

	serialNumber = new smChar[10];
	isOpened = false;
	calibrationData = new ADUDeviceCalibrationData();
	deviceData = new ADUDeviceData();
	ifstream reader;


	reader.open(calibrationFile,ios::in);

	if(!reader){

		cout<<" ADU Device Calibration Data File Could Not Be Loaded For Reading"<<endl;
		cout<<" Check the location for file : "<<deviceData<<endl;

	}

	smChar buffer[100];
	smInt i;
	string t;
	string s;
	while(!reader.eof()){

		reader.getline(buffer,50);

		t= buffer;

		i = t.find("Serial Number:");

		if(i!=string::npos)
		{

			i = t.find(":");
			s.assign(t,i+1,t.size());

			strcpy(serialNumber, s.c_str());

			//Debug 
			cout<<"Serial Number of this device is"<<serialNumber<<endl;
		}
		else{
			cout<<"Couldn't find the serial number for this ADU Device -- Check calibration file"<<endl;
		}

		s.clear();
		i = t.find("AN0MIN:");

		if(i!=string::npos)
		{

			i= t.find(":");

			s.assign(t,i+1,t.size());

			calibrationData->minValue1 = atoi(s.c_str());
		}

		s.clear();

		i = t.find("AN0MAX:");

		if(i!=string::npos)
		{

			i= t.find(":");

			s.assign(t,i+1,t.size());

			calibrationData->maxValue1 = atoi(s.c_str());
			

		}

		s.clear();
		i = t.find("AN1MIN:");

		if(i!=string::npos)
		{

			i= t.find(":");

			s.assign(t,i+1,t.size());

			calibrationData->minValue2 = atoi(s.c_str());
			

		}


		s.clear();

		i = t.find("AN1MAX:");

		if(i!=string::npos)
		{

			i= t.find(":");

			s.assign(t,i+1,t.size());

			calibrationData->maxValue2 = atoi(s.c_str());

		}

	}



	reader.close();

	cout<< "Calibration values are : "<<calibrationData->minValue1<<" " <<calibrationData->maxValue1<<" " <<calibrationData->minValue2<<" "<<calibrationData->maxValue2<<endl;


	if(openDevice(serialNumber)== SOFMIS_MSG_SUCCESS){
		cout<<"ADU USB Device Opened Successfully"<<endl;
	}
	else
		cout<<"Check the USB connection of the ADU device or the serial number: Couldn't initialize the device"<<endl;



	ADUpipe = new smPipe("ADU_Data",sizeof(ADUDeviceData),10);

	sw = 0;
	updateFlag = 0;


}

smADUInterface::~smADUInterface(){

closeDevice();

}

smInt smADUInterface::openDevice(char *serialNumber){


	deviceHandle = OpenAduDeviceBySerialNumber(serialNumber,0);

	//deviceHandle == OpenAduDeviceBySerialNumber("B02363",0);

	if( (*((int *)&deviceHandle ) ) >0 ){

		isOpened = true;

		return SOFMIS_MSG_SUCCESS;

	} else{
	
		isOpened = false;
		return SOFMIS_MSG_FAILURE;

	}


}



smInt smADUInterface::closeDevice(){

	CloseAduDevice(deviceHandle);

	return 0;

}



smInt* smADUInterface::readAnalogInputs(){

	return 0;

}




/// Support for two inputs
smInt smADUInterface::readAnalogInput(int channel){


	smChar *command;

	smChar data[8];

	if(channel == 0)
		command = "RUN00";
	else 
		command = "RUN10";

	WriteAduDevice(deviceHandle, command, 5, 0, 0);
	memset(data, 0, sizeof(data));
	ReadAduDevice(deviceHandle, data, 5, 0, 0);

	return atoi(data);

}


void smADUInterface::runDevice(){

	if(isOpened){
		
		for(smInt i =0; i<2; i++) deviceData->anValue[i] = readAnalogInput(i);

		updateFlag = !updateFlag;
		
		//deviceData->anValue[sw] = readAnalogInput(sw);
		//if(sw == 0) sw = 1;
		//else sw = 0;
	}
}

void smADUInterface::sendDataToPipe(){
	
	ADUDeviceData *pipeData;

	pipeData = (ADUDeviceData*)ADUpipe->beginWrite();

	pipeData->calibration[0] = calibrationData->minValue1;
	pipeData->calibration[1] = calibrationData->maxValue1;
	pipeData->calibration[2] = calibrationData->minValue2;
	pipeData->calibration[3] = calibrationData->maxValue2;

	pipeData->anValue[0] = deviceData->anValue[0];
	pipeData->anValue[1] = deviceData->anValue[1];

	//pipeData->anValue[0] = sampdata[0];
	//pipeData->anValue[1] = sampdata[10];

	pipeData->deviceOpen = isOpened;

	ADUpipe->endWrite(1);
	ADUpipe->acknowledgeValueListeners();
}

void smADUInterface::run(){

	while(1){

		
		//simTime->start();
		runDevice();
		

		sendDataToPipe();
		//printf("%f\n", simTime->now(SOFMIS_TIMER_INMILLISECONDS));
		// increase the number so that ADU device won't slow down the system
		//msleep(100);  //20

	}

}

/*class smADUReader: public QThread {

public:
	smADUInterface *USBInterface;

	

	smADUReader(){

		USBInterface = new smADUInterface("calibration.txt");
		

	}

	~smADUReader(){

	}
	
	void run(){


		while(1){

			USBInterface->runDevice();

			sleep(200);


		}


	}

};*/
//smInt write(void *Interfacehandle, smInt port, void *data){
//
//
//	if(port == 0){
//
//
//
//
//	return SOFMIS_MSG_SUCCESS;
//}
//
//smInt read(void *Interfacehandle, smInt port, void *data){
//	return SOFMIS_MSG_SUCCESS;
//
//}

#endif //WIN32 include
