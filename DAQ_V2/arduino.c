  #include <SPI.h>
#include <mcp2515.h>
struct can_frame canMsg;
struct can_frame canMsg1;
struct can_frame canMsg0;
struct can_frame canMsg3;
 int i =0;
 int ledpin = 3;
boolean buttonState;
boolean buttonStateBefore;
MCP2515 mcp2515(10);
int str[8];
int uart;


void setup() {
  pinMode(ledpin, OUTPUT);
  canMsg1.can_id  = 1349;
  canMsg1.can_dlc = 8;
  canMsg1.data[0] = 1;
  canMsg1.data[1] = 2;
  canMsg1.data[2] = 3;
  canMsg1.data[3] = 4;
  canMsg1.data[4] = 5;
  canMsg1.data[5] = 6;
  canMsg1.data[6] = 7;
  canMsg1.data[7] = 8;

  canMsg0.can_id  = 1351;
  canMsg0.can_dlc = 8;
  canMsg0.data[0] = 6;
  canMsg0.data[1] = 5;
  canMsg0.data[2] = 7;
  canMsg0.data[3] = 5;
  canMsg0.data[4] = 2;
  canMsg0.data[5] = 4;
  canMsg0.data[6] = 57;
  canMsg0.data[7] = 3;

  canMsg3.can_id  = 1350;
  canMsg3.can_dlc = 8;
  canMsg3.data[0] = 12;
  canMsg3.data[1] = 0x00;
  canMsg3.data[2] = 0x00;
  canMsg3.data[3] = 12;
  canMsg3.data[4] = 12;
  canMsg3.data[5] = 0x00;
  canMsg3.data[6] = 0x00;
  canMsg3.data[7] = 11;
  
  
  while (!Serial);
  Serial.begin(115200);
  
  mcp2515.reset();
  mcp2515.setBitrate(CAN_250KBPS,MCP_8MHZ);
  mcp2515.setNormalMode();
  
Serial.println("Example: Write to CAN");
Serial.println("------- CAN Read ----------");
Serial.println("ID  DLC   DATA");


}

void loop() {
  mcp2515.sendMessage(&canMsg1);
    delay(1);
  mcp2515.sendMessage(&canMsg3);
    delay(1);

  mcp2515.sendMessage(&canMsg0);
  delay(17);
}
