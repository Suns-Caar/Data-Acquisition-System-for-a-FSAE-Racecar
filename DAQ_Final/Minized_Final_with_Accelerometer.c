#include "xcanps.h"
#include "xparameters.h"
#include "xil_printf.h"
#include "xsdps.h"
//#include "xiicps.h"
#include "xuartps.h"
#include "xtime_l.h"
//#include "lis2ds12.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#define CAN_DEVICE_ID	XPAR_XCANPS_0_DEVICE_ID
#define TEST_BTR_SYNCJUMPWIDTH		1
#define TEST_BTR_SECOND_TIMESEGMENT	2
#define TEST_BTR_FIRST_TIMESEGMENT	15
#define TEST_BRPR_BAUD_PRESCALAR	19
#define IIC_DEVICE_ID		XPAR_XIICPS_0_DEVICE_ID
#define IIC_SLAVE_ADDR		0x55
#define IIC_SCLK_RATE		100000
#define MINIZED_MOTION_SENSOR_ADDRESS_SA0_HI  0x1D /* 0011101b for LIS2DS12 on MiniZed when SA0 is pulled high*/
#define TEST_BUFFER_SIZE 32
int CanPsPolledExample(u16 DeviceId);
void ReadMemory();
static int RecvFrame(XCanPs *InstancePtr);
XSdPs_Config * EMMC_Config;
XUartPs_Config *Config_0;
XUartPs Uart_PS_0;
XTime Time;
static XSdPs ps7_EMMC;
u8 Emmc_ExtCsd[1024];
u8 prevID = -1;
static u32 RxFrame[8];
u8 TelemetryFrame[10] = { 0 };
u8 Buf[60][10];
u8 Comp_Buf[50000000][10];
static XCanPs Can;
u32 addr = 0x05;
u32 tot = -1;
int msg = -1;
u8 flag = 0;
XIicPs Iic;		/**< Instance of the IIC Device */
u8 SendBuffer[TEST_BUFFER_SIZE];    /**< Buffer for Transmitting Data */
u8 RecvBuffer[TEST_BUFFER_SIZE];    /**< Buffer for Receiving Data */
float ACC_VAL[4];

/* ------------------------------------ ACCELEROMETER CODE----------------------------------*/
int IicPsMasterPolledExample()
{
	int Status;
	XIicPs_Config *Config;
	int Index;
	Config = XIicPs_LookupConfig(IIC_DEVICE_ID);
	if (NULL == Config) {
		return XST_FAILURE;
	}
	Status = XIicPs_CfgInitialize(&Iic, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	Status = XIicPs_SelfTest(&Iic);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	XIicPs_SetSClk(&Iic, IIC_SCLK_RATE);
	return XST_SUCCESS;
}
u8 LIS2DS12_WriteReg(u8 Reg, u8 *Bufp, u16 len)
{
	SendBuffer[0] = Reg;
	int Status;
	for (int ByteCount = 1;ByteCount <= len; ByteCount++)
	{
		SendBuffer[ByteCount] = Bufp[ByteCount-1];
	}
	Status = XIicPs_MasterSendPolled(&Iic, SendBuffer,	 (len+1), MINIZED_MOTION_SENSOR_ADDRESS_SA0_HI);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	while (XIicPs_BusIsBusy(&Iic)) {
		/* NOP */
	}

	return XST_SUCCESS;
}
u8 LIS2DS12_ReadReg(uint8_t Reg, uint8_t *Bufp, uint16_t len)
{
	int Status;
	SendBuffer[0] = Reg;
	Status = XIicPs_MasterSendPolled(&Iic, SendBuffer,1, MINIZED_MOTION_SENSOR_ADDRESS_SA0_HI);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	while (XIicPs_BusIsBusy(&Iic)) {
		/* NOP */
	}

	Status = XIicPs_MasterRecvPolled(&Iic, Bufp, len, MINIZED_MOTION_SENSOR_ADDRESS_SA0_HI);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	while (XIicPs_BusIsBusy(&Iic)) {
		/* NOP */
	}

	return XST_SUCCESS;
}
bool isSensorConnected()
{
	uint8_t who_am_i = 0;
	uint8_t send_byte;

	LIS2DS12_ReadReg(LIS2DS12_ACC_WHO_AM_I_REG, &who_am_i, 1);
	if (who_am_i != LIS2DS12_ACC_WHO_AM_I)
	{
		LIS2DS12_ReadReg(LIS2DS12_ACC_WHO_AM_I_REG, &who_am_i, 1);
	}
	send_byte = 0x00; //No auto increment
	LIS2DS12_WriteReg(LIS2DS12_ACC_CTRL2, &send_byte, 1);

	//Write 60h in CTRL1	// Turn on the accelerometer.  14-bit mode, ODR = 400 Hz, FS = 2g
	send_byte = 0x60;
	LIS2DS12_WriteReg(LIS2DS12_ACC_CTRL1, &send_byte, 1);

	//Enable interrupt
	send_byte = 0x01; //Acc data-ready interrupt on INT1
	LIS2DS12_WriteReg(LIS2DS12_ACC_CTRL4, &send_byte, 1);
	return true;
}
bool sensor_init()
{
	if (XST_SUCCESS != IicPsMasterPolledExample())
	{
		return false;
	}
	isSensorConnected();
	return true;
}

int u16_2s_complement_to_int(u16 word_to_convert)
{
	u16 result_16bit;
	int result_14bit;
	int sign;

	if (word_to_convert & 0x8000)
	{ //MSB is set, negative number
		Invert and add 1
		sign = -1;
		result_16bit = (~word_to_convert) + 1;
	}
	else
	{ //Positive number
		No change
		sign = 1;
		result_16bit = word_to_convert;
	}
	We are using it in 14-bit mode
	All data is left-aligned.  So convert 16-bit value to 14-but value
	result_14bit = sign * (int)(result_16bit >> 2);
	return(result_14bit);
}
void pollForAccel()
{
		int iacceleration_X;
		int iacceleration_Y;
		int iacceleration_Z;
		u8 read_value_LSB;
		u8 read_value_MSB;
		u16 accel_X;
		u16 accel_Y;
		u16 accel_Z;
		u8 accel_status;
		u8 data_ready;

		data_ready = 0;
		LIS2DS12_ReadReg(LIS2DS12_ACC_STATUS, &accel_status, 1);
		data_ready = accel_status & 0x01; //bit 0 = DRDY
		if (!data_ready)
		return false;
		LIS2DS12_ReadReg(LIS2DS12_ACC_OUT_X_L, &read_value_LSB, 1);
		LIS2DS12_ReadReg(LIS2DS12_ACC_OUT_X_H, &read_value_MSB, 1);
		accel_X = (read_value_MSB << 8) + read_value_LSB;
		iacceleration_X = u16_2s_complement_to_int(accel_X);

		Read Y:
		LIS2DS12_ReadReg(LIS2DS12_ACC_OUT_Y_L, &read_value_LSB, 1);
		LIS2DS12_ReadReg(LIS2DS12_ACC_OUT_Y_H, &read_value_MSB, 1);
		accel_Y = (read_value_MSB << 8) + read_value_LSB;
		iacceleration_Y = u16_2s_complement_to_int(accel_Y);

		//Read Z:
		LIS2DS12_ReadReg(LIS2DS12_ACC_OUT_Z_L, &read_value_LSB, 1);
		LIS2DS12_ReadReg(LIS2DS12_ACC_OUT_Z_H, &read_value_MSB, 1);
		accel_Z = (read_value_MSB << 8) + read_value_LSB;
		iacceleration_Z = u16_2s_complement_to_int(accel_Z);

		ACC_VAL[0]=(iacceleration_X*4*0.061*9.8)/1000;
		ACC_VAL[1]=(iacceleration_Y*4*0.061*9.8)/1000;
		ACC_VAL[2]=(iacceleration_Z*4*0.061*9.8)/1000;

		printf("\n X: %.3f m/s^2, Y: %.3f m/s^2, Z= %.3f m/s^2 \n", ACC_VAL[0],ACC_VAL[1],ACC_VAL[2]);

}

/* ------------------------------------ ACCELEROMETER CODE ENDS ----------------------------------------------------*/

#ifndef TESTAPP_GEN
int main() {
//	sensor_init();
	int Status;
	Status = CanPsPolledExample(CAN_DEVICE_ID);
	return XST_SUCCESS;
}
#endif
int CanPsPolledExample(u16 DeviceId) {

	int Status;
	XCanPs *CanInstPtr = &Can;
	XCanPs_Config *ConfigPtr;
	EMMC_Config = XSdPs_LookupConfig(XPAR_PS7_SD_1_DEVICE_ID);
	Status = XSdPs_CfgInitialize(&ps7_EMMC, EMMC_Config,EMMC_Config->BaseAddress);
	Status = XSdPs_MmcCardInitialize(&ps7_EMMC);
	Status = XSdPs_Change_ClkFreq(&ps7_EMMC, 50000000);
	Status = XSdPs_Select_Card(&ps7_EMMC);
	Status = XSdPs_SetBlkSize(&ps7_EMMC, XSDPS_BLK_SIZE_512_MASK);
	Status = XSdPs_Get_Mmc_ExtCsd(&ps7_EMMC, Emmc_ExtCsd);
	Config_0 = XUartPs_LookupConfig(XPAR_XUARTPS_0_DEVICE_ID);
	Status = XUartPs_CfgInitialize(&Uart_PS_0, Config_0, Config_0->BaseAddress);
	ConfigPtr = XCanPs_LookupConfig(DeviceId);
	if (CanInstPtr == NULL) {
		return XST_FAILURE;
	}
	Status = XCanPs_CfgInitialize(CanInstPtr, ConfigPtr, ConfigPtr->BaseAddr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	XCanPs_EnterMode(CanInstPtr, XCANPS_MODE_CONFIG);
	while (XCanPs_GetMode(CanInstPtr) != XCANPS_MODE_CONFIG);
	XCanPs_SetBaudRatePrescaler(CanInstPtr, TEST_BRPR_BAUD_PRESCALAR);
	XCanPs_SetBitTiming(CanInstPtr, TEST_BTR_SYNCJUMPWIDTH,
			TEST_BTR_SECOND_TIMESEGMENT, TEST_BTR_FIRST_TIMESEGMENT);
	XCanPs_EnterMode(CanInstPtr, XCANPS_MODE_NORMAL);
	while (XCanPs_GetMode(CanInstPtr) != XCANPS_MODE_NORMAL) {
		if (XCanPs_GetMode(CanInstPtr) == XCANPS_MODE_NORMAL) {
			break;
		}
	}
	print("------------CAN Reception starting now-------------- \n");
	while (1) {
		if(XCanPs_IsRxEmpty(CanInstPtr) == FALSE){
			Status=RecvFrame(CanInstPtr);
	}
		pollForAccel();
//		sleep(1);
	}

	return Status;
}

static int RecvFrame(XCanPs *InstancePtr) {

	u32 *FramePtr;
	int Status;
	u8 Index;
	u16 ID;
	u8 DLC;
	u8 d1, d2, d3, d4, d5, d6, d7, d8;
	Status = XCanPs_Recv(InstancePtr, RxFrame);
	if (Status == XST_SUCCESS) {
		tot++;
		msg++;
		if (msg > 51) {
			XSdPs_WritePolled(&ps7_EMMC, addr, 2, Buf);
			memset(Buf, 0, sizeof(Buf));
			msg = 0;
			addr++;
		}

	}
	FramePtr = (u32*) (&RxFrame[0]);
	ID = (u16) (FramePtr[0] >> 21);
	if (ID == 0) {
		XSdPs_WritePolled(&ps7_EMMC, addr, 2, Buf);
		XCanPs_Reset(InstancePtr);
		ReadMemory();
	}
	if (prevID == ID) {
		return;
	}
	DLC = (u8) (FramePtr[1] >> 24);
	DLC = DLC / ((u8) 16);
	d1 = (u8) (FramePtr[2]);
	d2 = (FramePtr[2] >> 8);
	FramePtr[2] = FramePtr[2] >> 8;
	d3 = FramePtr[2] >> 8;
	FramePtr[2] = FramePtr[2] >> 8;
	d4 = (u8) (FramePtr[2] >> 8);
	d5 = (u8) (FramePtr[3]);
	d6 = (FramePtr[3] >> 8);
	FramePtr[3] = FramePtr[3] >> 8;
	d7 = FramePtr[3] >> 8;
	FramePtr[3] = FramePtr[3] >> 8;
	d8 = (u8) (FramePtr[3] >> 8);
	xil_printf("\n CAN ID: %d and msg no: %d", ID, tot);
	Buf[msg][0] = ID;
	Buf[msg][1] = DLC;
	Buf[msg][2] = d1;
	Buf[msg][3] = d2;
	Buf[msg][4] = d3;
	Buf[msg][5] = d4;
	Buf[msg][6] = d5;
	Buf[msg][7] = d6;
	Buf[msg][8] = d7;
	Buf[msg][9] = d8;
	prevID = ID;
	return Status;
}

void ReadMemory() {
	xil_printf("\n ----READING COMPLETE DATA FROM MEMORY---- \n \n");
	memset(Buf, 0, sizeof(Buf));
	addr = 5;
	msg = 0;
	u8 RD_flag = 1;
	while (RD_flag) {
		XSdPs_ReadPolled(&ps7_EMMC, addr, 2, Buf);
		for (int i = 0; i < 52; i++) {
			if (Buf[i][0] != 2 && Buf[i][0] != 3 && Buf[i][0] != 1
					&& Buf[i][0] != 4 && Buf[i][0] != 5 && Buf[i][0] != 6) {
				RD_flag = 0;
				break;
			}
			for (int j = 0; j < 10; j++) {
				Comp_Buf[msg][j] = Buf[i][j];
			}
			msg++;
		}
		addr++;
		memset(Buf, 0, sizeof(Buf));
	}

	for (int i = 0; i < 50000000; i++) {
		if (Comp_Buf[i][0] == 0) {
			break;
		}
		xil_printf("Message Number:%d", i);
		for (int j = 0; j < 10; j++) {
			xil_printf("\t %d", Comp_Buf[i][j]);

		}
		xil_printf("\n");
	}
	xil_printf("---- DONE READING DATA FROM MEMORY----");

}

