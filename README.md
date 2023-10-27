# DAQ_FM_ECS

This Repository contains the codes for the Final Version of the Custom Data Acquisition System Based on Xilinx's FPGA SoC Avnet Minized, MINI M4-STM32, and Motec M400 ECU for the season of 2022-2023. 

Minized_noaccel Folder contains the Xilinx SDK project file while the Z_system_wrapper_hw_platform_0 folder is the hardware platform for the FPGA Board. 

This project describes the design and working of a Motorsport Data Acquisition/Logging System, Live Telemetry and Display system developed using the Controller Area Network (CAN) communication protocol as the backbone of the arrangement. A Formula One car hosts over a hundred sensors during each of its races. The Data Acquisition/logging System, although does not directly affect the car’s performance, is an indispensable system when it comes to improving and testing designs. Designers can validate their assumptions and calculations, real-time data during testing can be a safety indicator and it provides insight to the driver about the performance of the vehicle.

![image](https://github.com/Suns-Caar/DAQ_FM_ECS/assets/73470491/7cfd3a7d-25f7-4065-89e7-b0b28874ed84)



REFER TO Xilinx MASTER XDC and the Pinout mentioned here,for FPGA routing.

PINOUT:https://www.xilinx.com/support/packagefiles/z7packages/xc7z007sclg225pkg.txt

MASTER XDC:https://github.com/Avnet/hdl/blob/master/Boards/MINIZED/minized_pins.xdc
