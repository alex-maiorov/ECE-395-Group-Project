#include "painlessMesh.h"
#include "Button2.h"
#include <TFT_eSPI.h>

#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555

Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;

// uncomment only one of the following
#define WHITE
//#define RED
//#define GREEN

#ifdef WHITE
#define ROLE    "white"
#define VERSION "WHITE v1.0.0"
#define MESSAGE "White "
#endif
#ifdef RED
#define ROLE    "red"
#define VERSION "RED - 1.0.0"
#define MESSAGE "Red "
#endif
#ifdef GREEN
#define ROLE    "green"
#define VERSION "GREEN - 1.0.0"
#define MESSAGE "Green "
#endif
// TFT start
#ifndef TFT_DISPOFF
#define TFT_DISPOFF 0x28
#endif
#ifndef TFT_SLPIN
#define TFT_SLPIN   0x10
#endif
#define TFT_MOSI            19
#define TFT_SCLK            18
#define TFT_CS              5
#define TFT_DC              16
#define TFT_RST             23
#define TFT_BL          4   // Display backlight control pin
#define ADC_EN          14  //ADC_EN is the ADC detection enable port
#define ADC_PIN         34
#define BUTTON_1        35
#define BUTTON_2        0

TFT_eSPI tft = TFT_eSPI(135, 240); // Invoke custom library
// TFT end
Button2 btn1(BUTTON_1);
Button2 btn2(BUTTON_2);

bool calc_delay = false;
SimpleList<uint32_t> nodes;
uint32_t nsent=0;
char buff[512];


// User stub
void sendMessage(String msg) ; // Prototype so PlatformIO doesn't complain

Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage );

// Needed for painless library


void receivedCallback( uint32_t from, String &msg ) 
  {
  Serial.printf("Rx from %u <- %s\n", from, msg.c_str());

  if (strcmp(msg.c_str(),"GETRT") == 0)
    {
    mesh.sendBroadcast( mesh.subConnectionJson(true).c_str() );
    }
  else
    {
    sprintf(buff,"Rx:%s",msg.c_str());
    tft.drawString(buff, 0, 48);
    }
  
  }



void newConnectionCallback(uint32_t nodeId) 
  {
  Serial.printf("--> Start: New Connection, nodeId = %u\n", nodeId);
  Serial.printf("--> Start: New Connection, %s\n", mesh.subConnectionJson(true).c_str());
  }



void changedConnectionCallback() 
  {
  Serial.printf("Changed connections\n");

  nodes = mesh.getNodeList();
  Serial.printf("Num nodes: %d\n", nodes.size());
  Serial.printf("Connection list:");
  SimpleList<uint32_t>::iterator node = nodes.begin();
  while (node != nodes.end()) 
    {
    Serial.printf(" %u", *node);
    node++;
    }
  Serial.println();
  calc_delay = true;

  sprintf(buff,"Nodes:%d",nodes.size());
  tft.drawString(buff, 0, 32);
  }



void nodeTimeAdjustedCallback(int32_t offset) 
  {
  Serial.printf("Adjusted time %u Offset = %d\n", mesh.getNodeTime(),offset);
  }



void onNodeDelayReceived(uint32_t nodeId, int32_t delay)
  {
  Serial.printf("Delay from node:%u delay = %d\n", nodeId,delay);
  }




void sendMessage(String msg) {
  if (strcmp(msg.c_str(), "") == 0)
    {
    nsent++;
    msg = MESSAGE;
    //msg += mesh.getNodeId();
    msg += nsent;
    }
  mesh.sendBroadcast( msg );
  Serial.printf("Tx-> %s\n", msg.c_str());

  sprintf(buff,"Tx:%s",msg.c_str());
  tft.drawString(buff, 0, 64);

  if (calc_delay) 
    {
    SimpleList<uint32_t>::iterator node = nodes.begin();
    while (node != nodes.end()) 
      {
      mesh.startDelayMeas(*node);
      node++;
      }
    calc_delay = false;
    }

  taskSendMessage.setInterval( random( TASK_SECOND * 2, TASK_SECOND * 10 ));
}


void button_setup()
{
  // button dx
  btn1.setPressedHandler([](Button2 & b) 
    {
    sendMessage("GETRT");   // get routing table
    });
  
  // button sx
  btn2.setPressedHandler([](Button2 & b) 
    {
    sendMessage("");
    });
}


void setup() 
  {
  Serial.begin(115200);

  // TFT
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  //tft.setTextSize(4);
  tft.setTextColor(TFT_GREEN,TFT_BLACK);
  tft.setCursor(0, 0);
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(2);

  if (TFT_BL > 0) 
    { // TFT_BL has been set in the TFT_eSPI library in the User Setup file TTGO_T_Display.h
    pinMode(TFT_BL, OUTPUT); // Set backlight pin to output mode
    digitalWrite(TFT_BL, TFT_BACKLIGHT_ON); // Turn backlight on. TFT_BACKLIGHT_ON has been set in the TFT_eSPI library in the User Setup file TTGO_T_Display.h
    }
  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(TL_DATUM);
  tft.drawString(VERSION, 0, 0);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("LeftButton:", tft.width() / 2, tft.height() / 2 - 16);

  mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | MSG_TYPES | REMOTE ); // all types on except GENERAL
  mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages

  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  mesh.initOTAReceive(ROLE);

  //userScheduler.addTask( taskSendMessage );
  //taskSendMessage.enable();
  
  sprintf(buff,"Id:%d",mesh.getNodeId());
  tft.drawString(buff, 0, 16);

  button_setup();
  }



void loop() 
  {
  // it will run the user scheduler as well
  mesh.update();
  btn1.loop();
  btn2.loop();
  }
