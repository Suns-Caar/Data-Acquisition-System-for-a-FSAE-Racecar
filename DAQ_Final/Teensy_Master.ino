   #include <SD.h>
#include <SPI.h>
#include <FlexCAN_T4.h>
#include "USBHost_t36.h"
#include <EEPROM.h>
FlexCAN_T4<CAN2, RX_SIZE_256, TX_SIZE_16> Can0;
#define USBBAUD 115200
uint32_t baud = USBBAUD;
uint32_t format = USBHOST_SERIAL_8N1;
const int chipSelect = BUILTIN_SDCARD;
char filename[20];
char filename2[20];
char filename3[20];
int buffersize = 512;
byte sdbuffer[512];
int bufferIndex =0;
USBHost myusb;
USBHub hub1(myusb);
USBHub hub2(myusb);
USBHIDParser hid1(myusb);
USBHIDParser hid2(myusb);
USBHIDParser hid3(myusb);
int address =197;
unsigned int counter, filecounter;
unsigned long currentTimestamp;
int previous_millis, current_millis;
String Data;
String az;
int variablecount ;
int commacount;
int rawvalueRR;
int rawvalueRL;
float voltageRR, voltageRL;
USBSerial userial(myusb);  // works only for those Serial devices who transfer <=64 bytes (like T3.x, FTDI...)
USBDriver *drivers[] = {&hub1, &hub2, &hid1, &hid2, &hid3, &userial};
#define CNT_DEVICES (sizeof(drivers)/sizeof(drivers[0]))
const char * driver_names[CNT_DEVICES] = {"Hub1", "Hub2",  "HID1", "HID2", "HID3", "USERIAL1" };
bool driver_active[CNT_DEVICES] = {false, false, false, false};
File dataFile;
File CanFile;
File ADCFile;
static  bool counterUpdated = false; 
 void setup(void) {
 
  pinMode(13, OUTPUT);
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  for (int i = 0; i < 5; i++) {
    digitalWrite(2, HIGH);
    delayMicroseconds(50);
    digitalWrite(2, LOW);
    delayMicroseconds(50);
  }
  Serial.begin(115200); delay(400);
 digitalWrite(13, !digitalRead(13));  Can0.begin();
  Can0.setBaudRate(500000);
  Can0.setMaxMB(16);
  Can0.enableFIFO();
  Can0.enableFIFOInterrupt();
  Can0.onReceive(canSniff);
  Can0.mailboxStatus();
  Serial.println("\n\nUSB Host Testing - Serial");
  myusb.begin();
  //  Serial1.begin(115200);  // We will echo stuff Through Serial1...
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");
 
  getUniqueFilename(filename,"vector_%lu.csv");
    getUniqueFilename(filename2,"Front_%lu.csv");
    getUniqueFilename(filename3,"Rear_%lu.csv");
  File dataFile = SD.open(filename, FILE_WRITE);
  dataFile.println("TEST DATA STARTS FROM HERE");
}

void canSniff(const CAN_message_t &msg) {
    Serial.println("recieving");
  digitalWrite(13, !digitalRead(13));


  File CanFile = SD.open(filename2, FILE_WRITE);
  if (CanFile) {
    unsigned int SUS1 = (msg.buf[1] << 8) | msg.buf[0];
    unsigned int SUS2 = (msg.buf[3] << 8) | msg.buf[2];
    float susvoltage1 = (3.3/4096)*SUS1;float susvoltage2 = (3.3/4096)*SUS2;
   Serial.print("SUS1,"); Serial.print(susvoltage1);Serial.print(",SUS2,");
    Serial.print(susvoltage2);Serial.println();
    
    CanFile.print("SUS1,"); CanFile.print(susvoltage1);CanFile.print(",SUS2,");
    CanFile.print(susvoltage2);CanFile.println();
   
  } else {
    File CanFile = SD.open(filename, FILE_WRITE);
    // if the file didn't open, print an error:
    Serial.println("error opening CanFile.txt");

  }
  CanFile.close();

}

void loop() {
previous_millis = millis();
//if(current_millis - previous_millis >= 10){
  previous_millis = current_millis;
   File ADCFile = SD.open(filename3, FILE_WRITE);
   if(ADCFile){
//    Serial.println(current_millis-previous_millis);
    rawvalueRR = analogRead(A2);
    rawvalueRL = analogRead(A3);
    float voltageRR = rawvalueRR*(3.3/1023);
     float voltageRL = rawvalueRL*(3.3/1023);
    ADCFile.print(rawvalueRR);ADCFile.print(",");ADCFile.print(rawvalueRL);ADCFile.print(",");ADCFile.print(voltageRR);ADCFile.print(",");ADCFile.print(voltageRL);
//    
//    Serial.println();Serial.print("SUSRR,");Serial.print(voltageRR);Serial.print(",SUSRL,");Serial.print(voltageRL);
//    Serial.println();
    ADCFile.println();
    }else{
      File ADCFile = SD.open(filename3, FILE_WRITE);
      Serial.println("error opening ADCfile.csv");
      }
//  }

  Can0.events();
  myusb.Task();
  // Print out information about different devices.
  for (uint8_t i = 0; i < CNT_DEVICES; i++) {
    if (*drivers[i] != driver_active[i]) {
      if (driver_active[i]) {
        Serial.printf("*** Device %s - disconnected ***\n", driver_names[i]);
        driver_active[i] = false;
      } else {
        Serial.printf("*** Device %s %x:%x - connected ***\n", driver_names[i], drivers[i]->idVendor(), drivers[i]->idProduct());
        driver_active[i] = true;

        const uint8_t *psz = drivers[i]->manufacturer();
        if (psz && *psz) Serial.printf("  manufacturer: %s\n", psz);
        psz = drivers[i]->product();
        if (psz && *psz) Serial.printf("  product: %s\n", psz);
        psz = drivers[i]->serialNumber();
        if (psz && *psz) Serial.printf("  Serial: %s\n", psz);

        // If this is a new Serial device.
        if (drivers[i] == &userial) {
          // Lets try first outputting something to our USerial to see if it will go out...
          userial.begin(baud);
          Serial.println(format);
        }
      }
    }
  }

  if (Serial.available()) {

    userial.begin(baud, 8);

  }



  while (Serial1.available()) {
    //    Serial.println("Serial1 Available");
    Serial1.write(Serial1.read());
  }

  while (userial.available()) {
    //    Serial.println("USerial Available");
    //    Serial.write(userial.read());
    parse();

  }

}
int parse() {
  String Data = userial.readStringUntil('\n');
//  sdbuffer[]= Data;
//  Serial.println(Data);
  if (Data.startsWith("$VNISE")) {
    String tokens[16];
    int variablecount = 0;
    int commacount = 0;
    for (int i = 0; i < Data.length(); i++) {
      if (Data.charAt(i) == ',') {
        tokens[variablecount] = Data.substring(commacount + 0, i);
        variablecount++;
        commacount = i;
      }
      //        if (variablecount == 15) {
      //
      //          break;
      //        }
    }
        printBufferToSD(sdbuffer);
       Serial.print("Yaw: "); Serial.print(","); Serial.print("Pitch: "); Serial.print(","); Serial.print("Roll: "); Serial.print(","); Serial.print("Ax: ");
       Serial.print(",");  Serial.print("Ay: "); Serial.print(","); Serial.println("Az: ");
       Serial.print(tokens[1]);
       Serial.print(",");
       Serial.print(tokens[2]);
       Serial.print(",");
       Serial.print(tokens[3]);
       Serial.print(",");
       Serial.print(tokens[10]);
       Serial.print(",");
       Serial.print(tokens[11]);
       Serial.print(",");
       //        if(tokens[12] != "") {
       //           Serial.print("Az: ");
       Serial.println(tokens[12]);
//            }
           Serial.print("Vx: ");
           Serial.println(tokens[7]);
           Serial.print("Vy: ");
           Serial.println(tokens[8]);
           Serial.print("vz: ");
           Serial.println(tokens[9]);
    

  }

}

 
void getUniqueFilename(char* filename, const char* form) {
  
  if(counterUpdated == false){
     unsigned int counter = EEPROM.read(0);// Retrieve the counter from EEPROM
    Serial.println(counter);
    EEPROM.write(0,  counter++);
    counterUpdated = true; 
    }
  filecounter = millis();
    snprintf(filename, 20, form, filecounter);
  
}
void addArrayToBuffer(const char* arrayData) {
  size_t arrsize = strlen(Data)
  if (bufferIndex + arrsize > sdbufferSize) {
    printBufferToSD();
    bufferIndex = 0;
  }

  for (size_t i = 0; i < size; i++) {
    sdbuffer[bufferIndex] = (byte)arrayData[i];
    bufferIndex++;
  }
}
