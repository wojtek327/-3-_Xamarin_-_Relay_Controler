#include <SoftwareSerial.h>
#include <Wire.h>
#include <EEPROM.h>
#include "RTClib.h"
//-------------------------------------------
/* EEPROM Structure */
/* 
 *  Relay state: 0xA5 - Low State
 *               0xB9 - High State
 *  
 *  EEPROM[0] - Last State R1
 *  EEPROM[1] - Last State R2
 *  EEPROM[2] - Last State R3
 *  EEPROM[3] - Last State R4
 *  
 *  Time:
 *  EEPROM[4] - Relay 1 Time On/Off 0xA5/0xB9
 *  EEPROM[5] - HourStart Relay 1
 *  EEPROM[6] - HourStop Relay 1
 *  
 *  EEPROM[7] - Relay 2 Time On/Off 0xA5/0xB9
 *  EEPROM[8] - HourStart Relay 2
 *  EEPROM[9] - HourStop Relay 2
 *  
 *  EEPROM[10] - Relay 3 Time On/Off 0xA5/0xB9
 *  EEPROM[11] - HourStart Relay 3
 *  EEPROM[12] - HourStop Relay 3
 *  
 *  EEPROM[13] - Relay 4 Time On/Off 0xA5/0xB9
 *  EEPROM[14] - HourStart Relay 4
 *  EEPROM[15] - HourStop Relay 4
 */
/* Memmory map */
#define EEPROM_R1_STATE   0
#define EEPROM_R2_STATE   1
#define EEPROM_R3_STATE   2
#define EEPROM_R4_STATE   3

#define EEPROM_R1_TIME_STATE   4
#define EEPROM_R1_TIME_ON_H    5
#define EEPROM_R1_TIME_ON_M    6
#define EEPROM_R1_TIME_OFF_H   7
#define EEPROM_R1_TIME_OFF_M   8

#define EEPROM_R2_TIME_STATE   9
#define EEPROM_R2_TIME_ON_H    10
#define EEPROM_R2_TIME_ON_M    11
#define EEPROM_R2_TIME_OFF_H   12
#define EEPROM_R2_TIME_OFF_M   13

#define EEPROM_R3_TIME_STATE   14
#define EEPROM_R3_TIME_ON_H    15
#define EEPROM_R3_TIME_ON_M    16
#define EEPROM_R3_TIME_OFF_H   17
#define EEPROM_R3_TIME_OFF_M   18

#define EEPROM_R4_TIME_STATE   19
#define EEPROM_R4_TIME_ON_H    20
#define EEPROM_R4_TIME_ON_M    21
#define EEPROM_R4_TIME_OFF_H   22
#define EEPROM_R4_TIME_OFF_M   23

#define EEPROM_LAST_WRITED_ADDR 23
//-------------------------------------------
#define SOFT_UART_RX 11
#define SOFT_UART_TX 10
//-------------------------------------------
#define RELAY1_PIN 2
#define RELAY2_PIN 3
#define RELAY3_PIN 4
#define RELAY4_PIN 5
//-------------------------------------------
#define ON_STATE    LOW
#define OFF_STATE   HIGH
//-------------------------------------------
#define MEM_RELAY_ON_STATE  0xB9
#define MEM_RELAY_OFF_STATE 0xA5
//-------------------------------------------
//define PIN11 and PIN10 as RX and TX
SoftwareSerial I2CBT(SOFT_UART_RX,SOFT_UART_TX);
RTC_DS1307 rtc;
//-------------------------------------------
bool checkReceiveData(byte recSingleData);
void setDataByBuffer(byte dataBuffer[]);
bool checkRecRelBuf(byte dataBuffer[], char dataToCheck);
bool checkIfWriteCommand(byte dataBuffer[]);
bool checkIfReadCommand(byte dataBuffer[]);
void sendReadResponseFrame();
void setRelayStateBaseOnEEPROMData();
int BitShiftCombine(unsigned char x_high, unsigned char x_low);
void clearEEPROMData(byte relayNumber);
//-------------------------------------------
char recData[128];
int dataLoop = 0;
//-------------------------------------------
void setup()
{
  Serial.begin(9600);
  I2CBT.begin(9600);

  Serial.println("Uart Enabled");
  
  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  pinMode(RELAY3_PIN, OUTPUT);
  pinMode(RELAY4_PIN, OUTPUT);

  digitalWrite(RELAY1_PIN, OFF_STATE);
  digitalWrite(RELAY2_PIN, OFF_STATE);
  digitalWrite(RELAY3_PIN, OFF_STATE);
  digitalWrite(RELAY4_PIN, OFF_STATE);
  
  Serial.println("Pinout Setted");
  //setRelayStateBaseOnEEPROMData();
}
//-------------------------------------------
void loop()
{
  char recSingleData;
  int insize = 0;
  char character;
  
  if ((insize=(I2CBT.available())) > 0)
  {
     Serial.write(recSingleData=char(I2CBT.read()));
     recData[dataLoop] = recSingleData;
     dataLoop++;
  }
  
  if(checkReceiveData(recSingleData))
  {
    dataLoop = 0;
    setDataByBuffer(recData);
  }
}
//-------------------------------------------
void checkTime()
{
   DateTime now = rtc.now(); 
}
//-------------------------------------------
bool checkReceiveData(byte recSingleData)
{
  /* Check if data frame receive */
  if(recSingleData == '\n')
  {
    return true;
  }
  
  return false;
}
//-------------------------------------------
void setDataByBuffer(byte dataBuffer[])
{
  if(checkRecRelBuf(dataBuffer, '1'))
  {
    prepareBufferRelay1(dataBuffer);
  }
  else if(checkRecRelBuf(dataBuffer, '2'))
  {
    prepareBufferRelay2(dataBuffer);
  }
  else if(checkRecRelBuf(dataBuffer, '3'))
  {
    prepareBufferRelay3(dataBuffer);
  }
  else if(checkRecRelBuf(dataBuffer, '4'))
  {
    prepareBufferRelay4(dataBuffer);
  }
  else if(checkIfReadCommand(dataBuffer))
  {
    Serial.println("Read Data");
    sendReadResponseFrame();
  }
  else if(checkIfWriteCommand(dataBuffer))
  {
    Serial.println("Write Data");
    writeDataToEEPROM(dataBuffer);
  }
  else
  {
    Serial.println("Receive Different Data");
  }
}
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
void prepareBufferRelay1(byte dataBuffer[])
{
  if(dataBuffer[4] == 'n')
  {
     Serial.println("Turn On Relay 1");
     digitalWrite(RELAY1_PIN, ON_STATE);
     EEPROM.write(EEPROM_R1_STATE, MEM_RELAY_ON_STATE);
  }
  else if(dataBuffer[4] == 'f' && dataBuffer[5] == 'f')
  {
    Serial.println("Turn Off Relay 1");
    digitalWrite(RELAY1_PIN, OFF_STATE);
    EEPROM.write(EEPROM_R1_STATE, MEM_RELAY_OFF_STATE);
  }
}
void prepareBufferRelay2(byte dataBuffer[])
{
  if(dataBuffer[4] == 'n')
  {
    Serial.print("Turn On Relay 2");
    digitalWrite(RELAY2_PIN, ON_STATE);
    EEPROM.write(EEPROM_R2_STATE, MEM_RELAY_ON_STATE);
  }
  else if(dataBuffer[4] == 'f' && dataBuffer[5] == 'f')
  {
    Serial.println("Turn Off Relay 2");
    digitalWrite(RELAY2_PIN, OFF_STATE);
    EEPROM.write(EEPROM_R2_STATE, MEM_RELAY_OFF_STATE);
  }
}
void prepareBufferRelay3(byte dataBuffer[])
{
  if(dataBuffer[4] == 'n')
  {
    Serial.println("Turn On Relay 3");
    digitalWrite(RELAY3_PIN, ON_STATE);
    EEPROM.write(EEPROM_R3_STATE, MEM_RELAY_ON_STATE);
  }
  else if(dataBuffer[4] == 'f' && dataBuffer[5] == 'f')
  {
    Serial.println("Turn Off Relay 3");
    digitalWrite(RELAY3_PIN, OFF_STATE);
    EEPROM.write(EEPROM_R3_STATE, MEM_RELAY_OFF_STATE);
  }
}
void prepareBufferRelay4(byte dataBuffer[])
{
  if(dataBuffer[4] == 'n')
  {
    Serial.println("Turn On Relay 4");
    digitalWrite(RELAY4_PIN, ON_STATE);
    EEPROM.write(EEPROM_R4_STATE, MEM_RELAY_ON_STATE);
  }
  else if(dataBuffer[4] == 'f' && dataBuffer[5] == 'f')
  {
    Serial.println("Turn Off Relay 4");
    digitalWrite(RELAY4_PIN, OFF_STATE);
    EEPROM.write(EEPROM_R4_STATE, MEM_RELAY_OFF_STATE);
  }
}
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
bool checkRecRelBuf(byte dataBuffer[], char dataToCheck)
{
  if(dataBuffer[1] == 'R' && dataBuffer[2] == dataToCheck && 
     dataBuffer[3] == 'O')
  {
    return true;
  }
  return false;
}
//-------------------------------------------
bool checkIfWriteCommand(byte dataBuffer[])
{
  if(dataBuffer[1] == 'W' && dataBuffer[2] == 'r' && dataBuffer[3] == 'i') 
  {
    return true;
  }
  return false;
}
//-------------------------------------------
bool writeDataToEEPROM(byte receiveBuffer[])
{
  byte bufferPosition = 0;
  /* Decode frame */
  /* If there is no data to write, then time will be xx:xx */
  /* Example buffer bWriR1x;R212:23;13:00;R3x;R4x;*/
  if(receiveBuffer[bufferPosition + 4] == 'R' && receiveBuffer[bufferPosition + 5] == '1')
  {
    Serial.println("Data Relay1");
    if(receiveBuffer[bufferPosition + 6] == 'x')
    {
      bufferPosition = 8;
      clearEEPROMData(1);
    }
    else
    {
      EEPROM.write(EEPROM_R1_TIME_STATE, 0xA5);
      /* There is data to write to EEPROM buffer position 6 7 9 10 12 13 15 16 (hh:mm;hh:mm)*/
      String dataConvert = String(receiveBuffer[6] - '0') + String(receiveBuffer[7] - '0');
      byte timeR1SH = (byte)dataConvert.toInt();
      Serial.println(timeR1SH);
      EEPROM.write(EEPROM_R1_TIME_ON_H, timeR1SH);
      Serial.println("Write1 OK");
      //-------------------------------------------------
      dataConvert = "";
      dataConvert = String(receiveBuffer[9] - '0') + String(receiveBuffer[10] - '0'); 
      timeR1SH = (byte)(dataConvert.toInt());
      Serial.println(timeR1SH);
      EEPROM.write(EEPROM_R1_TIME_ON_M, timeR1SH);
      Serial.println("Write2 OK");
      //-------------------------------------------------
      dataConvert = "";
      dataConvert = String(receiveBuffer[12] - '0') + String(receiveBuffer[13] - '0'); 
      timeR1SH = (byte)(dataConvert.toInt());
      Serial.println(timeR1SH);
      EEPROM.write(EEPROM_R1_TIME_OFF_H, timeR1SH);
      Serial.println("Write3 OK");
      //-------------------------------------------------
      dataConvert = String(receiveBuffer[15] - '0') + String(receiveBuffer[16] - '0'); 
      timeR1SH = (byte)(dataConvert.toInt());
      Serial.println(timeR1SH);
      EEPROM.write(EEPROM_R1_TIME_OFF_M, timeR1SH);
      Serial.println("Write4 OK");
      //-------------------------------------------------
      bufferPosition = 18;
    }
  }

  if(receiveBuffer[bufferPosition] == 'R' && receiveBuffer[bufferPosition+1] == '2')
  {
    Serial.println("Data Relay2");
    if(receiveBuffer[bufferPosition + 2] == 'x')
    {
      bufferPosition += 4;
      clearEEPROMData(2);
    }
    else
    {
      EEPROM.write(EEPROM_R2_TIME_STATE, 0xA5);
      /* There is data to write to EEPROM buffer position 20 21 23 24 26 27 29 30 (hh:mm;hh:mm)*/
      String dataConvert = String(receiveBuffer[bufferPosition+2] - '0') + String(receiveBuffer[bufferPosition+3] - '0');
      byte timeR1SH = (byte)(dataConvert.toInt());
      Serial.println(timeR1SH);
      EEPROM.write(EEPROM_R2_TIME_ON_H, timeR1SH);
      Serial.println("Write1 OK");
      //-------------------------------------------------
      dataConvert = String(receiveBuffer[bufferPosition + 5] - '0') + String(receiveBuffer[bufferPosition + 6] - '0'); 
      timeR1SH = (byte)(dataConvert.toInt());
      Serial.println(timeR1SH);
      EEPROM.write(EEPROM_R2_TIME_ON_M, timeR1SH);
      Serial.println("Write2 OK");
      //-------------------------------------------------
      dataConvert = String(receiveBuffer[bufferPosition + 8] - '0') + String(receiveBuffer[bufferPosition + 9] - '0'); 
      timeR1SH = (byte)(dataConvert.toInt());
      Serial.println(timeR1SH);
      EEPROM.write(EEPROM_R2_TIME_OFF_H, timeR1SH);
      Serial.println("Write3 OK");
      //-------------------------------------------------
      dataConvert = String(receiveBuffer[bufferPosition + 11] - '0') + String(receiveBuffer[bufferPosition + 12] - '0'); 
      timeR1SH = (byte)(dataConvert.toInt());
      Serial.println(timeR1SH);
      EEPROM.write(EEPROM_R2_TIME_OFF_M, timeR1SH);
      Serial.println("Write4 OK");
      //-------------------------------------------------
      bufferPosition = 32;
    }
  }

  if(receiveBuffer[bufferPosition] == 'R' && receiveBuffer[bufferPosition + 1] == '3')
  {
    Serial.println("Data Relay3");
    if(receiveBuffer[bufferPosition + 2] == 'x')
    {
      bufferPosition += 4;
      clearEEPROMData(3);
    }
    else
    {
      EEPROM.write(EEPROM_R3_TIME_STATE, 0xA5);
      /* There is data to write to EEPROM buffer position 35 36 38 39 41 42 44 45 (hh:mm;hh:mm)*/
      String dataConvert = String(receiveBuffer[bufferPosition+2] - '0') + String(receiveBuffer[bufferPosition+3] - '0');
      byte timeR1SH = (byte)(dataConvert.toInt());
      Serial.println(timeR1SH);
      EEPROM.write(EEPROM_R3_TIME_ON_H, timeR1SH);
      Serial.println("Write1 OK");
      //-------------------------------------------------
      dataConvert = String(receiveBuffer[bufferPosition + 5] - '0') + String(receiveBuffer[bufferPosition + 6] - '0'); 
      timeR1SH = (byte)(dataConvert.toInt());
      Serial.println(timeR1SH);
      EEPROM.write(EEPROM_R3_TIME_ON_M, timeR1SH);
      Serial.println("Write2 OK");
      //-------------------------------------------------
      dataConvert = String(receiveBuffer[bufferPosition + 8] - '0') + String(receiveBuffer[bufferPosition + 9] - '0'); 
      timeR1SH = (byte)(dataConvert.toInt());
      Serial.println(timeR1SH);
      EEPROM.write(EEPROM_R3_TIME_OFF_H, timeR1SH);
      Serial.println("Write3 OK");
      //-------------------------------------------------
      dataConvert = String(receiveBuffer[bufferPosition + 11] - '0') + String(receiveBuffer[bufferPosition + 12] - '0'); 
      timeR1SH = (byte)(dataConvert.toInt());
      Serial.println(timeR1SH);
      EEPROM.write(EEPROM_R3_TIME_OFF_M, timeR1SH);
      Serial.println("Write4 OK");
      //-------------------------------------------------
      bufferPosition = 46;
    }
  }

  if(receiveBuffer[bufferPosition] == 'R' && receiveBuffer[bufferPosition + 1] == '4')
  {
    Serial.println("Data Relay4");
    if(receiveBuffer[bufferPosition + 2] == 'x')
    {
      clearEEPROMData(4);
    }
    else
    {
      EEPROM.write(EEPROM_R4_TIME_STATE, 0xA5);
      /* There is data to write to EEPROM buffer position 35 36 38 39 41 42 44 45 (hh:mm;hh:mm)*/
      String dataConvert = String(receiveBuffer[bufferPosition+2] - '0') + String(receiveBuffer[bufferPosition+3] - '0');
      byte timeR1SH = (byte)(dataConvert.toInt());
      Serial.println(timeR1SH);
      EEPROM.write(EEPROM_R4_TIME_ON_H, timeR1SH);
      Serial.println("Write1 OK");
      //-------------------------------------------------
      dataConvert = String(receiveBuffer[bufferPosition + 5] - '0') + String(receiveBuffer[bufferPosition + 6] - '0'); 
      timeR1SH = (byte)(dataConvert.toInt());
      Serial.println(timeR1SH);
      EEPROM.write(EEPROM_R4_TIME_ON_M, timeR1SH);
      Serial.println("Write2 OK");
      //-------------------------------------------------
      dataConvert = String(receiveBuffer[bufferPosition + 8] - '0') + String(receiveBuffer[bufferPosition + 9] - '0'); 
      timeR1SH = (byte)(dataConvert.toInt());
      Serial.println(timeR1SH);
      EEPROM.write(EEPROM_R4_TIME_OFF_H, timeR1SH);
      Serial.println("Write3 OK");
      //-------------------------------------------------
      dataConvert = String(receiveBuffer[bufferPosition + 11] - '0') + String(receiveBuffer[bufferPosition + 12] - '0'); 
      timeR1SH = (byte)(dataConvert.toInt());
      Serial.println(timeR1SH);
      EEPROM.write(EEPROM_R4_TIME_OFF_M, timeR1SH);
      Serial.println("Write4 OK");
    }
  }
}

void clearEEPROMData(byte relayNumber)
{
  if(relayNumber == 1)
  {
      EEPROM.write(EEPROM_R1_TIME_STATE, 0xB9);
      EEPROM.write(EEPROM_R1_TIME_ON_H, 0x00);
      EEPROM.write(EEPROM_R1_TIME_ON_M, 0x00);
      EEPROM.write(EEPROM_R1_TIME_OFF_H, 0x00);
      EEPROM.write(EEPROM_R1_TIME_OFF_M, 0x00);
  }
  else if(relayNumber == 2)
  {
      EEPROM.write(EEPROM_R2_TIME_STATE, 0xB9);
      EEPROM.write(EEPROM_R2_TIME_ON_H, 0x00);
      EEPROM.write(EEPROM_R2_TIME_ON_M, 0x00);
      EEPROM.write(EEPROM_R2_TIME_OFF_H, 0x00);
      EEPROM.write(EEPROM_R2_TIME_OFF_M, 0x00);   
  }
  else if(relayNumber == 3)
  {
      EEPROM.write(EEPROM_R3_TIME_STATE, 0xB9);
      EEPROM.write(EEPROM_R3_TIME_ON_H, 0x00);
      EEPROM.write(EEPROM_R3_TIME_ON_M, 0x00);
      EEPROM.write(EEPROM_R3_TIME_OFF_H, 0x00);
      EEPROM.write(EEPROM_R3_TIME_OFF_M, 0x00);
  }
  else if(relayNumber == 4)
  {
      EEPROM.write(EEPROM_R4_TIME_STATE, 0xB9);
      EEPROM.write(EEPROM_R4_TIME_ON_H, 0x00);
      EEPROM.write(EEPROM_R4_TIME_ON_M, 0x00);
      EEPROM.write(EEPROM_R4_TIME_OFF_H, 0x00);
      EEPROM.write(EEPROM_R4_TIME_OFF_M, 0x00);
  }
}
//-------------------------------------------
void setRelayStateBaseOnEEPROMData()
{
  byte table[4] = { 0x00 };

  for(int loop = 0; loop<4; loop++)
  {
    table[loop] = EEPROM.read(loop);
  }

  //Readed data table[0] - relay1 ... table[3] - relay4
  if(table[0] == MEM_RELAY_ON_STATE)        { digitalWrite(RELAY1_PIN, ON_STATE); }
  else if(table[0] == MEM_RELAY_OFF_STATE)  { digitalWrite(RELAY1_PIN, OFF_STATE); }

  if(table[1] == MEM_RELAY_ON_STATE)        { digitalWrite(RELAY2_PIN, ON_STATE); }
  else if(table[0] == MEM_RELAY_OFF_STATE)  { digitalWrite(RELAY2_PIN, OFF_STATE); }

  if(table[2] == MEM_RELAY_ON_STATE)        { digitalWrite(RELAY3_PIN, ON_STATE); }
  else                                      { digitalWrite(RELAY4_PIN, OFF_STATE); }

  if(table[3] == MEM_RELAY_ON_STATE)        { digitalWrite(RELAY4_PIN, ON_STATE); }
  else                                      { digitalWrite(RELAY4_PIN, OFF_STATE); }
}
//-------------------------------------------
void readDataFromEEPROM(byte* pointerTable)
{ 
  for(int loop = 0; loop < EEPROM_LAST_WRITED_ADDR + 1; loop++)
  {
    pointerTable[loop] = EEPROM.read(loop);
  }
}
//-------------------------------------------
bool checkIfReadCommand(byte dataBuffer[])
{
  if(dataBuffer[1] == 'R' && dataBuffer[2] == 'e' && dataBuffer[3] == 'a' && dataBuffer[4] == 'd')
  {
    return true;
  }
  return false;
}
//-------------------------------------------
void sendReadResponseFrame()
{
  byte readedDataFromEEPROM[EEPROM_LAST_WRITED_ADDR + 2] = { 0x00 };

  readDataFromEEPROM(&readedDataFromEEPROM[0]);

  readedDataFromEEPROM[EEPROM_LAST_WRITED_ADDR + 1] = 'q';

  for(int loop = 0; loop < EEPROM_LAST_WRITED_ADDR + 2; loop++)
  {
      Serial.println(readedDataFromEEPROM[loop], HEX);
  }

  /* Send readed parametrs */
  I2CBT.write(readedDataFromEEPROM, EEPROM_LAST_WRITED_ADDR);
}

