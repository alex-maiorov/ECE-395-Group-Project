///*
//  Reading lat and long via UBX binary commands - no more NMEA parsing!
//  By: Nathan Seidle
//  SparkFun Electronics
//  Date: January 3rd, 2019
//  License: MIT. See license file for more information but you can
//  basically do whatever you want with this code.
//
//  This example shows how to query a u-blox module for its lat/long/altitude. We also
//  turn off the NMEA output on the I2C port. This decreases the amount of I2C traffic 
//  dramatically.
//
//  Note: Long/lat are large numbers because they are * 10^7. To convert lat/long
//  to something google maps understands simply divide the numbers by 10,000,000. We 
//  do this so that we don't have to use floating point numbers.
//
//  Leave NMEA parsing behind. Now you can simply ask the module for the datums you want!
//
//  Feel like supporting open source hardware?
//  Buy a board from SparkFun!
//  ZED-F9P RTK2: https://www.sparkfun.com/products/15136
//  NEO-M8P RTK: https://www.sparkfun.com/products/15005
//  SAM-M8Q: https://www.sparkfun.com/products/15106
//
//  Hardware Connections:
//  Plug a Qwiic cable into the GNSS and a BlackBoard
//  If you don't have a platform with a Qwiic connection use the SparkFun Qwiic Breadboard Jumper (https://www.sparkfun.com/products/14425)
//  Open the serial monitor at 115200 baud to see the output
//*/
//

///////////////////////////////////////////////////////////

#include "Wire.h" //Needed for I2C to GNSS
#include <SparkFun_u-blox_GNSS_v3.h>
#include <string.h>
#include <bits/stdc++.h>
SFE_UBLOX_GNSS myGNSS;
long lastTime = 0; //Simple local timer. Limits amount if I2C traffic to u-blox module.

String lat = "0";
String lon = "0";
String alt = "0";
String SIV = "0";


#include "painlessMesh.h"
#define   MESH_PORT       5555
Scheduler userScheduler;
painlessMesh  mesh;
void sendMessage();
Task taskSendMessage(TASK_SECOND, TASK_FOREVER, &sendMessage);
void sendMessage() {
  String msg = "GPS Node, Long: " + lon + " (degrees * 10^-7), Lat: " + lat + ", SIV: " + SIV;
  mesh.sendBroadcast(msg);
  taskSendMessage.setInterval( random(TASK_SECOND, TASK_SECOND*5));
}
void receivedCallback( uint32_t from, String &msg ) {
  Serial.printf("Received from %u msg = %s\n", from, msg.c_str());
}
void newConnectionCallback(uint32_t nodeId) {
    Serial.printf("New Connection, nodeId = %u\n", nodeId);
}
void changedConnectionCallback() {
  Serial.printf("Changed connections\n");
}
void nodeTimeAdjustedCallback(int32_t offset) {
    Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
}
void setup() {
  Serial.begin(115200);
  while(!Serial); //Wait for user to open terminal
  Serial.println("ECE395 Demo");

  Wire.begin(6,7);
  //myGNSS.enableDebugging(); // Uncomment this line to enable helpful debug messages on Serial
  if (myGNSS.begin(Wire) == false) //Connect to the u-blox module using Wire port
  {
    Serial.println(F("u-blox GNSS not detected at default I2C address. Please check wiring. Freezing."));
    while (1);
  }
  myGNSS.setI2COutput(COM_TYPE_UBX); //Set the I2C port to output UBX only (turn off NMEA noise)
  myGNSS.saveConfigSelective(VAL_CFG_SUBSEC_IOPORT); //Save (only) the communications port settings to flash and BBR

  
  mesh.setDebugMsgTypes(ERROR | STARTUP);
  mesh.init("whateverYouLike", "somethingSneaky", &userScheduler, MESH_PORT);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  userScheduler.addTask(taskSendMessage);
  taskSendMessage.enable();
}
void loop() {
  mesh.update();
  //Query module only every second. Doing it more often will just cause I2C traffic.
  //The module only responds when a new position is available
  if (millis() - lastTime > 1000)
  {
    lastTime = millis(); //Update the timer
    
      std::string lat = std::to_string(myGNSS.getLatitude());

//    Serial.print(F("Lat: "));
//    Serial.print(latitude);

      std::string lon = std::to_string(15); //myGNSS.getLongitude()

//    Serial.print(F(" Long: "));
//    Serial.print(longitude);
//    Serial.print(F(" (degrees * 10^-7)"));

      std::string alt = std::to_string(myGNSS.getAltitude());

//    Serial.print(F(" Alt: "));
//    Serial.print(altitude);
//    Serial.print(F(" (mm)"));

      std::string SIV = std::to_string(myGNSS.getSIV());

//    Serial.print(F(" SIV: "));
//    Serial.print(SIV);

//    Serial.println();
  }
}

  
