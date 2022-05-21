#include "xcanps.h"
#include "xparameters.h"
#include "xil_printf.h"
#define CAN_DEVICE_ID	XPAR_XCANPS_0_DEVICE_ID
#define TEST_BTR_SYNCJUMPWIDTH		3
#define TEST_BTR_SECOND_TIMESEGMENT	2
#define TEST_BTR_FIRST_TIMESEGMENT	15
#define TEST_BRPR_BAUD_PRESCALAR	29

int CanPsPolledExample(u16 DeviceId);
static int RecvFrame(XCanPs *InstancePtr);
int total =0;
static u32 RxFrame[8];
static XCanPs Can;
#ifndef TESTAPP_GEN
int main()
{
	int Status;
	xil_printf("CAN example starting now");
	Status = CanPsPolledExample(CAN_DEVICE_ID);
	return XST_SUCCESS;
}
#endif
int CanPsPolledExample(u16 DeviceId)
{
	int Status;
	XCanPs *CanInstPtr = &Can;
	XCanPs_Config *ConfigPtr;
	ConfigPtr = XCanPs_LookupConfig(DeviceId);
	if (CanInstPtr == NULL) {
		return XST_FAILURE;
	}
	Status = XCanPs_CfgInitialize(CanInstPtr,
					ConfigPtr,
					ConfigPtr->BaseAddr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	XCanPs_EnterMode(CanInstPtr, XCANPS_MODE_CONFIG);
	while(XCanPs_GetMode(CanInstPtr) != XCANPS_MODE_CONFIG);
	XCanPs_SetBaudRatePrescaler(CanInstPtr, TEST_BRPR_BAUD_PRESCALAR);
	XCanPs_SetBitTiming(CanInstPtr, TEST_BTR_SYNCJUMPWIDTH,
				TEST_BTR_SECOND_TIMESEGMENT,

				TEST_BTR_FIRST_TIMESEGMENT);
	XCanPs_EnterMode(CanInstPtr, XCANPS_MODE_NORMAL);
	while(XCanPs_GetMode(CanInstPtr)!= XCANPS_MODE_NORMAL){
		if(XCanPs_GetMode(CanInstPtr)== XCANPS_MODE_NORMAL){
		break;}}
	print("------------CAN Reception starting now--------------");
	while(1){

		if(XCanPs_IsRxEmpty(CanInstPtr) == FALSE){
	Status=RecvFrame(CanInstPtr);
//	printf("%d",Status
//			);
//	sleep(1);
		}
	}

	return Status;
}
static int RecvFrame(XCanPs *InstancePtr)
{
	u32 *FramePtr;
	int Status;
	u8 Index;
	u16 ID;

	u8 a,b,c,d,DLC;
	u8 d1,d2,d3,d4,d5,d6,d7,d8;
	Status = XCanPs_Recv(InstancePtr, RxFrame);
	total++;
//		print("\n Message received");
		FramePtr = (u32*)(&RxFrame[0]);
		ID=(u16)(FramePtr[0]>>21);
		DLC=(u8)(FramePtr[1]>>24);
		DLC=DLC/((u8)16);
		d1=(u8)(FramePtr[2]);
		d2=(FramePtr[2]>>8);
		FramePtr[2]=FramePtr[2]>>8;
		d3=FramePtr[2]>>8;
		FramePtr[2]=FramePtr[2]>>8;
		d4=(u8)(FramePtr[2]>>8);
		d5=(u8)(FramePtr[3]);
		d6=(FramePtr[3]>>8);
		FramePtr[3]=FramePtr[3]>>8;
		d7=FramePtr[3]>>8;
		FramePtr[3]=FramePtr[3]>>8;
		d8=(u8)(FramePtr[3]>>8);
//		if(ID=2){
		xil_printf("\n CAN Message: %d: - %d %d ",ID, d1, d2);

//		}
//		xil_printf("\n Data bytes are \n %d ,%d ",d1,d2);

	return Status;
}

