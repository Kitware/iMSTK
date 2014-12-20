#include "smCore/smConfig.h"

#ifndef SIMMEDTK_OPERATINGSYSTEM_LINUX
#include <iostream>
#include <fstream>
#include <cassert>
#include "smExternalDevices/smNIUSB6008Interface.h"

#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else

int32 CVICALLBACK DoneCallback(TaskHandle taskHandle, int32 status, void *callbackData);
int32 CVICALLBACK EveryNCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData);

smNIUSB6008Interface::smNIUSB6008Interface(int VBLaST_Task_ID){

	int32  error=0;
	char errBuff[2048]={'\0'};
	NI_error = 0;
	for(smInt i=0;i<2048;i++) NI_errBuff[i] = '\0';
	initCount = 0;
	NI_on = false;
	count = 0;
	taskHandle=0;
	taskID = VBLaST_Task_ID;

	getToolCalibrationData();
	setTool();

	minValue[0] = -1.35;	minValue[1] = -1.35;
	maxValue[0] = 3.60;		maxValue[1] = 3.60;

	invRange[0] = 1.0/(maxValue[0]- minValue[0]);
	invRange[1] = 1.0/(maxValue[1]- minValue[1]);

	/*********************************************/
	// DAQmx Configure Code
	/*********************************************/
	initNI_Error(DAQmxCreateTask("",&taskHandle));

	switch(VBLaST_Task_ID){
		case 1:	// peg transfer: dissect, dissect
			//0 1
			initNI_Error(DAQmxCreateAIVoltageChan(taskHandle,"Dev1/ai0","",DAQmx_Val_RSE,-10.0,10.0,DAQmx_Val_Volts,NULL));
			initNI_Error(DAQmxCreateAIVoltageChan(taskHandle,"Dev1/ai1","",DAQmx_Val_RSE,-10.0,10.0,DAQmx_Val_Volts,NULL));
			break;
		case 2:	// pattern cutting: dissect, shear
			//0 2
			initNI_Error(DAQmxCreateAIVoltageChan(taskHandle,"Dev1/ai0","",DAQmx_Val_RSE,-10.0,10.0,DAQmx_Val_Volts,NULL));
			initNI_Error(DAQmxCreateAIVoltageChan(taskHandle,"Dev1/ai2","",DAQmx_Val_RSE,-10.0,10.0,DAQmx_Val_Volts,NULL));
			break;
		case 3: // ligating loop: ligating loop, shear, grasper
			//2 3 4
			initNI_Error(DAQmxCreateAIVoltageChan(taskHandle,"Dev1/ai2","",DAQmx_Val_RSE,-10.0,10.0,DAQmx_Val_Volts,NULL));
			initNI_Error(DAQmxCreateAIVoltageChan(taskHandle,"Dev1/ai3","",DAQmx_Val_RSE,-10.0,10.0,DAQmx_Val_Volts,NULL));
			initNI_Error(DAQmxCreateAIVoltageChan(taskHandle,"Dev1/ai4","",DAQmx_Val_RSE,-10.0,10.0,DAQmx_Val_Volts,NULL));
			break;
		case 4: // intracorporeal suturing: needle driver, needle driver
			// 5 6
			initNI_Error(DAQmxCreateAIVoltageChan(taskHandle,"Dev1/ai5","",DAQmx_Val_RSE,-10.0,10.0,DAQmx_Val_Volts,NULL));
			initNI_Error(DAQmxCreateAIVoltageChan(taskHandle,"Dev1/ai6","",DAQmx_Val_RSE,-10.0,10.0,DAQmx_Val_Volts,NULL));
			break;
		case 5:	// extracorporeal suturing: needle driver, needle driver, knot pusher
			// 5 6 7
			initNI_Error(DAQmxCreateAIVoltageChan(taskHandle,"Dev1/ai5","",DAQmx_Val_RSE,-10.0,10.0,DAQmx_Val_Volts,NULL));
			initNI_Error(DAQmxCreateAIVoltageChan(taskHandle,"Dev1/ai6","",DAQmx_Val_RSE,-10.0,10.0,DAQmx_Val_Volts,NULL));
			initNI_Error(DAQmxCreateAIVoltageChan(taskHandle,"Dev1/ai7","",DAQmx_Val_RSE,-10.0,10.0,DAQmx_Val_Volts,NULL));	//DAQmx_Val_Cfg_Default
			break;
		case 6:
			initNI_Error(DAQmxCreateAIVoltageChan(taskHandle,"Dev1/ai2","",DAQmx_Val_RSE,-10.0,10.0,DAQmx_Val_Volts,NULL));	  // stapler
			initNI_Error(DAQmxCreateAIVoltageChan(taskHandle,"Dev1/ai3","",DAQmx_Val_RSE,-10.0,10.0,DAQmx_Val_Volts,NULL));	 //grasper

		default:
			break;
	}

	initNI_Error(DAQmxCfgSampClkTiming(taskHandle,"",1000.0,DAQmx_Val_Rising,DAQmx_Val_ContSamps,1000));
	initNI_Error(DAQmxRegisterEveryNSamplesEvent(taskHandle,DAQmx_Val_Acquired_Into_Buffer,10,0,EveryNCallback,this));
	initNI_Error(DAQmxRegisterDoneEvent(taskHandle,0,DoneCallback,this));

	/*********************************************/
	// DAQmx Start Code
	/*********************************************/
	initNI_Error(DAQmxStartTask(taskHandle));

	if(initCount==0){
		printf("NI DAQ USB-6008 is working \n");
		NI_on = true;
	}
	else{
		printf("NI DAQ USB-6008 is not working \n");
		smInt count=0;
		while(count<1000){count++;}
		NI_on = false;
	}

	NIUSB6008pipe = new smPipe("NIUSB6008_Data",sizeof(NIUSB6008Data),10);
	aveData[0] = aveData[1] = 0.0;
}

/// \brief
void smNIUSB6008Interface::getToolCalibrationData(){

	FILE *fp_cali;
	fp_cali = fopen("config/toolCalibrationNIUSB6008.txt", "r");
	if (NULL == fp_cali) {
		printf("NI DAQ USB-6008 configuration file not found: config/toolCalibrationNIUSB6008.txt");
		assert(fp_cali);
	}
	fscanf(fp_cali, "%d\n", &nbrRegTool);
	regTool = new toolData[nbrRegTool];

	for(smInt i=0; i<nbrRegTool; i++){
		for(smInt j=0; j<5; j++){
			fscanf(fp_cali, "%c", &regTool[i].type[j]);
		}
		fscanf(fp_cali, " %f %f\n", &regTool[i].min, &regTool[i].max);
	}
	fclose(fp_cali);
}

/// \brief
void smNIUSB6008Interface::setTool()
{

	/*
	 #AI    |    type
	--------------------
	  0     |    DIS_0
	  1     |    DIS_1
	  2     |    SHE_0
	  3     |    LIG_0
	  4     |    GRA_0
	  5     |    NEE_0
	  6     |    NEE_1
	  7     |    KNO_0
	*/
	smInt i, j, k, n;
	FILE *fp;
	fp = fopen("config/toolSetting.txt", "r");
	if (NULL == fp){
		printf("Tool settings file not found: config/toolSetting.txt");
		assert(fp);
	}

	nbrTotalChannel = 8;
	for(i=0; i<nbrTotalChannel; i++){
		fscanf(fp, "%d ", &k);
		for(j=0; j<5; j++){
			fscanf(fp, "%c", &installedTool[i].type[j]);
		}
		fscanf(fp, "\n");
	}
	fclose(fp);

	//to minimize the number of AI channels to be used for maximizing sampling rate
	switch(taskID){
		case 1:	// peg transfer: dissect, dissect
			//0 1
			nbrActiveChannel = 2;
			break;
		case 2:	// pattern cutting: dissect, shear
			//0 2
			nbrActiveChannel = 2;
			activeChannel[0] = 0;
			activeChannel[1] = 2;
			getToolData(nbrActiveChannel, activeChannel);
			break;
		case 3: // ligating loop: ligating loop, shear, grasper
			//2 3 4
			nbrActiveChannel = 3;
			activeChannel[0] = 2;
			activeChannel[1] = 3;
			activeChannel[2] = 4;
			getToolData(nbrActiveChannel, activeChannel);
			break;
		case 4: // intracorporeal suturing: needle driver, needle driver
			// 5 6
			nbrActiveChannel = 2;
			break;
		case 5:	// extracorporeal suturing: needle driver, needle driver, knot pusher
			// 5 6 7
			nbrActiveChannel = 3;
			break;
		case 6:		//NOTES
			nbrActiveChannel  = 2;
			activeChannel[0] = 2;
			activeChannel[1] = 3;
			getToolData(nbrActiveChannel, activeChannel);

		default:
			break;
	}
}

/// \brief
void smNIUSB6008Interface::getToolData(smInt nc, smInt *ac){

	smInt i, j, k;
	smInt cc=0;

	for(i=0; i<nc; i++){
		for(j=0;j<nbrRegTool;j++){
			k = ac[i];
			if(installedTool[k].type[0] == regTool[j].type[0] &&
				installedTool[k].type[1] == regTool[j].type[1] &&
				installedTool[k].type[2] == regTool[j].type[2] &&
				installedTool[k].type[3] == regTool[j].type[3] &&
				installedTool[k].type[4] == regTool[j].type[4])
			{
				installedTool[k].min = regTool[j].min;
				installedTool[k].max = regTool[j].max;
				installedTool[k].invRange = 1.0/(installedTool[k].max - installedTool[k].min);
				cc++;
				break;
			}
		}
	}

	if(cc<nc){
		printf("\n\ncheck tool serial numbers and setting\n");
	}
}

/// \brief
void smNIUSB6008Interface::initNI_Error(int32 error){
	if(error<0){
		if( DAQmxFailed(error) )
			DAQmxGetExtendedErrorInfo(NI_errBuff,2048);
		if( taskHandle!=0 ) {
			/*********************************************/
			// DAQmx Stop Code
			/*********************************************/
			DAQmxStopTask(taskHandle);
			DAQmxClearTask(taskHandle);
		}
		if( DAQmxFailed(error) )
			printf("DAQmx Error: %s\n",NI_errBuff);
		initCount++;
	}
}

/// \brief
smNIUSB6008Interface::~smNIUSB6008Interface(){

	DAQmxStopTask(taskHandle);
	DAQmxClearTask(taskHandle);

	delete NIUSB6008pipe;
	delete [] regTool;
}

/// \brief
void smNIUSB6008Interface::sendDataToPipe(){

	NIUSB6008Data *pipeData;
	pipeData = (NIUSB6008Data*)NIUSB6008pipe->beginWrite();
	smFloat tF;
	smInt cid;
	pipeData->on = NI_on;

	for(smInt i=0;i<nbrActiveChannel;i++){
		cid = activeChannel[i];
		tF = (aveData[i] - installedTool[cid].min) * installedTool[cid].invRange;
		if(tF<0.0) tF=0.0;
		if(tF>1.0) tF=1.0;
		pipeData->value[i] = tF;
	}

	NIUSB6008pipe->endWrite(1);
	NIUSB6008pipe->acknowledgeValueListeners();
}

/// \brief
void smNIUSB6008Interface::run(){

	while(1){
		sendDataToPipe();
	}
}

/// \brief
int32 CVICALLBACK EveryNCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData)
{
	int32 error=0;
	char errBuff[2048]={'\0'};
	int32 read=0;

	smNIUSB6008Interface  *NIUSB6008Interface = static_cast<smNIUSB6008Interface *> (callbackData);

	/*********************************************/
	// DAQmx Read Code
	/*********************************************/

	if(NIUSB6008Interface->taskID==1 || NIUSB6008Interface->taskID==2 || NIUSB6008Interface->taskID==4 || NIUSB6008Interface->taskID==6){
		DAQmxErrChk (DAQmxReadAnalogF64(taskHandle, 10, 10.0, DAQmx_Val_GroupByChannel, NIUSB6008Interface->sampdata, 20, &read, NULL));
	}

	DAQmxErrChk (DAQmxReadAnalogF64(taskHandle, 10, 10.0, DAQmx_Val_GroupByChannel, NIUSB6008Interface->sampdata, 10 * NIUSB6008Interface->nbrActiveChannel, &read, NULL));

	if( read>0 ) {
		if(read==10){
			NIUSB6008Interface->aveData[0] = NIUSB6008Interface->aveData[1] = NIUSB6008Interface->aveData[2] = 0.0;
			smInt i, j;
			for(i=0;i<10;i++){
				for(j=0; j<NIUSB6008Interface->nbrActiveChannel; j++){
					NIUSB6008Interface->aveData[j] += NIUSB6008Interface->sampdata[i + j * 10];
				}
			}

			for(i=0;i<NIUSB6008Interface->nbrActiveChannel;i++){
				NIUSB6008Interface->aveData[i]/=10.0;
			}
		}
	}

Error:
	if( DAQmxFailed(error) ) {
		DAQmxGetExtendedErrorInfo(errBuff,2048);
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
		DAQmxStopTask(taskHandle);
		DAQmxClearTask(taskHandle);
		printf("DAQmx Error: %s\n",errBuff);
	}
	return 0;
}

/// \brief
int32 CVICALLBACK DoneCallback(TaskHandle taskHandle, int32 status, void *callbackData){

	int32 error=0;
	char errBuff[2048]={'\0'};

	// Check to see if an error stopped the task.
	DAQmxErrChk (status);

Error:
	if( DAQmxFailed(error) ) {
		DAQmxGetExtendedErrorInfo(errBuff,2048);
		DAQmxClearTask(taskHandle);
		printf("DAQmx Error: %s\n",errBuff);
	}
	return 0;
}

#endif //ifndef SIMMEDTK_OPERATINGSYSTEM_LINUX
