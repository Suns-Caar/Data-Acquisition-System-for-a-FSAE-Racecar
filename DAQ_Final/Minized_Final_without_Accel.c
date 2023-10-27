#include "xcanps.h"
	#include "xparameters.h"
	#include "xil_printf.h"
	#include "xsdps.h"
	#include "xuartps.h"
	#define TEST_BUFFER_SIZE	10
	#define CAN_DEVICE_ID	XPAR_XCANPS_0_DEVICE_ID
	#define TEST_BTR_SYNCJUMPWIDTH		1
	#define TEST_BTR_SECOND_TIMESEGMENT	2
	#define TEST_BTR_FIRST_TIMESEGMENT	15
	#define TEST_BRPR_BAUD_PRESCALAR	4
	int CanPsPolledExample(u16 DeviceId);
	void ReadMemory();
	static int RecvFrame(XCanPs *InstancePtr);
	XSdPs_Config * EMMC_Config;

	XUartPs_Config *Config_0;
	XUartPs Uart_PS_0;
	static XSdPs ps7_EMMC;
	u8 Emmc_ExtCsd[1024];
	u8 prevID=-1;
	static u32 RxFrame[8];
	u8 Buf[60][10];
	u8 Comp_Buf[50000000][10];
	static XCanPs Can;
	u32 addr=0x05;
	u32 tot=-1;
	#ifndef TESTAPP_GEN
	int msg=-1;
	u8 flag=0;
	int main()
	{
		int Status;
		Status = CanPsPolledExample(CAN_DEVICE_ID);
		return XST_SUCCESS;
	}
	#endif
	int CanPsPolledExample(u16 DeviceId)
	{
		int Status;
		XCanPs *CanInstPtr = &Can;
		XCanPs_Config *ConfigPtr;
		EMMC_Config= XSdPs_LookupConfig(XPAR_PS7_SD_1_DEVICE_ID);

		Status = XSdPs_CfgInitialize(&ps7_EMMC, EMMC_Config, EMMC_Config->BaseAddress);
		Status=XSdPs_MmcCardInitialize(&ps7_EMMC);
		Status=XSdPs_Change_ClkFreq(&ps7_EMMC,50000000);
		Status=XSdPs_Select_Card(&ps7_EMMC);
		Status=XSdPs_SetBlkSize(&ps7_EMMC,XSDPS_BLK_SIZE_512_MASK);
		Status=XSdPs_Get_Mmc_ExtCsd(&ps7_EMMC,Emmc_ExtCsd);

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
				printf("done");

			break;}}
		printf("------------CAN Reception starting now-------------- \n");
//		ReadMemory();
		while(1){

			if(XCanPs_IsRxEmpty(CanInstPtr) == FALSE){
				Status=RecvFrame(CanInstPtr);
				printf("in recieve");

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
		u8 DLC;
		u8 d1,d2,d3,d4,d5,d6,d7,d8;
		Status = XCanPs_Recv(InstancePtr, RxFrame);

			tot++;
			msg++;
			if(msg>51){
				XSdPs_WritePolled(&ps7_EMMC, addr, 2, Buf);
				memset(Buf, 0, sizeof(Buf));
				msg=0;
				addr++;
			}


			FramePtr = (u32*)(&RxFrame[0]);
			ID=(u16)(FramePtr[0]>>21);
			if(tot == 500000){
				XSdPs_WritePolled(&ps7_EMMC, addr, 2, Buf);
				XCanPs_Reset(InstancePtr);
				ReadMemory();
			}
			else {
				printf("65536");
				printf("\n CAN ID: %d %d %d %d %d %d %d %d %d",ID,d1,d2,d3,d4,d5,d6,d7,d8);
			}
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

			Buf[msg][0]=ID -1350;
			Buf[msg][1]=DLC;
			Buf[msg][2]=d1;
			Buf[msg][3]=d2;
			Buf[msg][4]=d3;
			Buf[msg][5]=d4;
			Buf[msg][6]=d5;
			Buf[msg][7]=d6;
			Buf[msg][8]=d7;
			Buf[msg][9]=d8;


			return Status;
	}

	void ReadMemory(){
		xil_printf("\n ----READING COMPLETE DATA FROM MEMORY---- \n \n");
		memset(Buf, 0, sizeof(Buf));
		addr=5;
		msg=0;
		u8 RD_flag=1;
		while(RD_flag){
			memset(Buf, 0, sizeof(Buf));
			XSdPs_ReadPolled(&ps7_EMMC, addr, 2, Buf);
			for(int i=0;i<52;i++){
//				if(Buf[i][0]!=1 && Buf[i][0]!=2 && Buf[i][0]!=3 && Buf[i][0]!=4 && Buf[i][0]!=5 && Buf[i][0]!=6  && Buf[i][0]!=7 && Buf[i][0]!=8 && Buf[i][0]!=9){
//					RD_flag=0;
//					break;
//				}
				xil_printf("Message Number:%d  ",msg);
				for(int j=0;j<10;j++){
//					Comp_Buf[msg][j]=Buf[i][j];
					xil_printf("\t %d",Buf[i][j]);
				}msg++;
				xil_printf("\n");
				}
				addr++;

		}
				for(int i=0;i<50000000;i++){
					if(Comp_Buf[i][0]==0){
							break;
					}
					xil_printf("Message Number:%d",i);
					for(int j=0;j<10;j++){
						xil_printf("\t %d",Comp_Buf[i][j]);

					}
					xil_printf("\n");
					}
					xil_printf("---- DONE READING DATA FROM MEMORY----");


	}
