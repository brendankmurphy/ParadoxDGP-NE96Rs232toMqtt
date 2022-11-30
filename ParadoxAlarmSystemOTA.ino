#include <FS.h>   
//#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266SSDP.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <DNSServer.h>
#include <PubSubClient.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>

#define firmware "DIGIPLEX_0.0.1"

#define mqtt_server       "192.168.1.1"
#define mqtt_port         "1883"
#define mqtt_user         ""
#define mqtt_password     "" 

#define Hostname          "paradoxdCTL" //not more than 15

#define timezone 2.0 //for setdate command

#define Stay_Arm  0x01
#define Stay_Arm2 0x02
#define Sleep_Arm 0x03
#define Full_Arm 0x04
#define Disarm  0x05
#define Bypass 0x10
#define PGMon 0x32
#define PGMoff 0x33

#define MessageLength 37

#define LED LED_BUILTIN

//if 1 uses d13 d15 for rx/tx 0 uses default rx/tx
//Default is to use onboard RX/TX 
#define Serial_Swap 0

#define Hassio 1 // 1 enables 0 disables Hassio-Openhab support
#define HomeKit 0 // enables homekit topic
#define SendAllE0events 1 //If you need all events set to 1 else 0 

//If you need event decriptions set to 1 else 0 Can cause slow downs on heavy systems.
//Can also be enabled by sending sendeventdescriptions=1 to in topic.
//Enable it here if you want it enabled after a reboot
bool SendEventDescriptions = 1;

/*
HomeKit id 
Characteristic.SecuritySystemCurrentState.STAY_ARM = 0;
Characteristic.SecuritySystemCurrentState.AWAY_ARM = 1;
Characteristic.SecuritySystemCurrentState.NIGHT_ARM = 2;
Characteristic.SecuritySystemCurrentState.DISARMED = 3;
Characteristic.SecuritySystemCurrentState.ALARM_TRIGGERED = 4;
*/

bool TRACE = 0;
bool OTAUpdate = 1;


 char *root_topicOut = "paradox/out";
 char *root_topicStatus = "paradox/status";
 char *root_topicIn = "paradox/in";
 char *root_topicHassioArm = "paradox/ha/Arm";
 char *root_topicHassio = "paradox/ha";
 char *root_topicArmHomekit = "paradox/HomeKit";
// char *root_topicDebug = "paradoxdCTL/debug";
 
//root_topicArmStatus

WiFiClient espClient;
// client parameters
PubSubClient client(espClient);

bool shouldSaveConfig = false;
bool ResetConfig = false;
bool PanelConnected =false;
bool PanelError = false;
bool RunningCommand=false;
bool JsonParseError=false;
 
char inData[38]; // Allocate some space for the string
byte pindex = 0; // Index into array; where to store the character

long lastReconnectAttempt = 0;
long armStatusDelay =0;


ESP8266WebServer HTTP(80);

struct inPayload
{
  byte PcPasswordFirst2Digits;
  byte PcPasswordSecond2Digits;
  byte Command;
  byte Subcommand;
 } ;
 

 typedef struct {
     int intArmStatus;
     String stringArmStatus;
     int sent;
 } paradoxArm;

 paradoxArm hassioStatus;
 
 paradoxArm homekitStatus;
 

void setup() {
  pinMode(LED, OUTPUT);
  
  WiFi.mode(WIFI_STA);

  
  Serial.begin(19200);
  Serial.flush(); // Clean up the serial buffer in case previous junk is there
  if (Serial_Swap)
  {
    Serial.swap();
  }

  Serial1.begin(115200);
  Serial1.flush();
  Serial1.setDebugOutput(true);
  trc(F("serial monitor is up"));
  serial_flush_buffer();

  

  trc(F("Running MountFs"));
  mountfs();

  setup_wifi();
  StartSSDP();
  
  ArduinoOTA.setHostname(Hostname);
  ArduinoOTA.begin();
  trc("Finished wifi setup");
  delay(1500);
  
  char readymsg[64];
  sprintf(readymsg, "{\"firmware\":\"SYSTEM %s\"}", firmware);
  sendCharMQTT(root_topicStatus,readymsg,false);
  lastReconnectAttempt = 0;
  serial_flush_buffer();
    
}

void loop() {
   readSerial();  
   
   if ( (inData[0] & 0xF0) != 0xE0 && (inData[0] & 0xF0) != 0x40 && (inData[0] & 0xF0) != 0x50 && (inData[0] & 0xF0) != 0x30 && (inData[0] & 0xF0) != 0x70)
    {
      trc(F("start serial_flush_buffer"));
      serial_flush_buffer();
      // DEBUG message over MQTT 
      //sendMQTT(root_topicOut, "{\"debug\":\"Flushed serial buffer\"}" , false);
      
    }
}

byte checksumCalculate(byte checksum) {
    while (checksum > 255) {
      checksum = checksum - (checksum / 256) * 256;
    }
  return checksum & 0xFF;
}

void StartSSDP(){
  if (WiFi.waitForConnectResult() == WL_CONNECTED) {

    Serial1.printf("Starting HTTP...\n");
    HTTP.on("/index.html", HTTP_GET, []() {
      HTTP.send(200, "text/html", getpage());
    });
    HTTP.on("/", HTTP_GET, []() {
      HTTP.send(200, "text/plain", Hostname);
    });

    HTTP.on("/description.xml", HTTP_GET, []() {
      SSDP.schema(HTTP.client());
    });
    HTTP.begin();

    Serial1.printf("Starting SSDP...\n");
    SSDP.setSchemaURL(F("description.xml"));
    SSDP.setDeviceType(F("upnp:rootdevice"));
    SSDP.setHTTPPort(80);
    SSDP.setName(Hostname);
    SSDP.setSerialNumber(WiFi.macAddress());
    SSDP.setURL(String("http://") + WiFi.localIP().toString().c_str() +"/index.html");
    SSDP.setModelName(F("ESP8266Wemos"));
    SSDP.setModelNumber(firmware);
    SSDP.setModelURL(F("https://github.com/maragelis/ParadoxRs232toMqtt"));
    SSDP.setManufacturer(F("PM ELECTRONICS"));
    SSDP.setManufacturerURL(F("https://github.com/maragelis/"));
    SSDP.begin();

    if (!MDNS.begin(Hostname)) {
    trc(F("Error setting up MDNS responder!"));
    while (1) {
      delay(1000);
    }
  }
    trc(F("mDNS responder started"));

  

  // Add service to MDNS-SD
    MDNS.addService("http", "tcp", 80);

    trc(F("Ready!\n"));
  }
}

void updateArmStatus(byte event, byte sub_event){
  bool datachanged = false;
  if (event == 18)
  {
        hassioStatus.stringArmStatus = "triggered";
        datachanged=true;
        sendMQTT(root_topicHassioArm,hassioStatus.stringArmStatus, true);
  }
  else if (event == 4)
  {
    if (sub_event == 3)
    {
      datachanged=true;
      hassioStatus.stringArmStatus = "armed_home";
      sendMQTT(root_topicHassioArm,hassioStatus.stringArmStatus, true);
    }
  }
  else if (event == 10 && hassioStatus.stringArmStatus != "armed_home" )
  {
    if (sub_event == 2)
    {
      datachanged=true;
      hassioStatus.stringArmStatus = "armed_away";
      sendMQTT(root_topicHassioArm,hassioStatus.stringArmStatus, true);
    }
  }
  else if (event == 12 && hassioStatus.stringArmStatus != "armed_home" )
  {
    if (sub_event == 1)
    {
      datachanged=true;
      hassioStatus.stringArmStatus = "armed_away";
      sendMQTT(root_topicHassioArm,hassioStatus.stringArmStatus, true);
    }
  }
  else if (event == 14)
  {
    if (sub_event == 2)
    {
      datachanged=true;
      hassioStatus.stringArmStatus = "disarmed";
      sendMQTT(root_topicHassioArm,hassioStatus.stringArmStatus, true);
    }
  }  
        
}

void sendArmStatus(){
  char output[128];
  StaticJsonBuffer<128> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
        if (Hassio)
        {
          sendMQTT(root_topicHassioArm,hassioStatus.stringArmStatus, true);  
        }
        if (HomeKit)
        {
          root["Armstatus"]=homekitStatus.intArmStatus;
          root["ArmStatusD"]=homekitStatus.stringArmStatus ;
          root.printTo(output);
          sendCharMQTT(root_topicArmHomekit,output, false); 
        }
}


void processMessage( byte event, byte sub_event, String dummy ){
  if ((Hassio || HomeKit) && (event == 4 || event == 10 || event == 12 || event == 14 || event == 18))
  {
    updateArmStatus(event,sub_event); 
  }

  //Dont send the arm event now send it on next message, because it might be updated to sleep or stay.
  if ((Hassio || HomeKit) &&  (event != 2 and sub_event != 12) )  
  {  
    if (homekitStatus.sent != homekitStatus.intArmStatus)
    {
      sendArmStatus();
      homekitStatus.sent = homekitStatus.intArmStatus;
      }
  }
 
  if ((Hassio || HomeKit) && (event == 29))
  {
      sendArmStatus();
      homekitStatus.sent = homekitStatus.intArmStatus;
  }
 
  if ((Hassio ) && (event == 1 || event == 0))
  {
    char ZoneTopic[80];
    String zone = String(root_topicHassio) + "/zone";
    zone.toCharArray(ZoneTopic, 80);
    zone = String(ZoneTopic) + String(sub_event);
    zone.toCharArray(ZoneTopic, 80);

    String zonestatus = event==1?"ON":"OFF";

    sendMQTT(ZoneTopic, zonestatus, true);    
  }
  
  if ((HomeKit ) && (event == 1 || event == 0))
  {
    char output[128];
    StaticJsonBuffer<128> jsonBuffer;
    JsonObject& homekitmsg = jsonBuffer.createObject();
    homekitmsg["zone"]=sub_event;
    dummy.trim();
    homekitmsg["zoneName"]=String(dummy);
    homekitmsg["state"]=event==1?true:false;
    homekitmsg.printTo(output);
    
    sendCharMQTT(root_topicArmHomekit,output,false); 
  }

  if (SendAllE0events)
  {
    char outputMQ[256];
    StaticJsonBuffer<256> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    root["event"]=event;
    root["sub_event"]=sub_event;
    if (SendEventDescriptions)
    {
      root["sub_eventD"]=getSubEvent(event,sub_event);
      root["eventD"]=getEvent(event);
    }

    if (root["event"]==14 && root["sub event"]==2) // Disarm event ... arming from panel generates E/SE 4/3, 12/4, 10/2
    {
      sendMQTT(root_topicHassioArm,"disarmed", true);
    }
    
    root["data"]=dummy;
    root.printTo(outputMQ);
    
    sendCharMQTT(root_topicOut,outputMQ,false); 
  }
}

void sendMQTT(String topicNameSend, String dataStr,bool  retain){
    handleMqttKeepAlive();
    char topicStrSend[40];
    topicNameSend.toCharArray(topicStrSend,26);
    char dataStrSend[200];
    dataStr.toCharArray(dataStrSend,200);
    boolean pubresult = client.publish(topicStrSend,dataStrSend ,retain);
    if (TRACE)
     {
      Serial1.print("Sent:");
      Serial1.print( "\"" + dataStr + "\"");
      Serial1.print(" to Topic:");
      Serial1.println(topicNameSend);
      Serial1.print("with pubresult :");
      Serial1.println(pubresult);
    }
}

void sendCharMQTT(char* topic, char* data , bool retain){
  handleMqttKeepAlive();
  if (TRACE)
  {
    Serial1.print("Sending MQmessage to topic: ");
    Serial1.println(topic);
    Serial1.print("With data: ");
    Serial1.println(data);
  }
  boolean pubresult = client.publish(topic, data, retain);
  
}

void readSerial(){
  while (Serial.available()<37  )  
  { 
    while (RunningCommand)
    {
      yield();
   }
      
      if (OTAUpdate)
      {
        ArduinoOTA.handle();
      }
      
      HTTP.handleClient();
      handleMqttKeepAlive();
      
  }                            
  {
    
    trc("Reading main loop");
    pindex=0;
  
    while(pindex < 37) // Paradox packet is 37 bytes 
    {
      inData[pindex++]=Serial.read();  
    } 
    inData[++pindex]=0x00; // Make it print-friendly

    if ((inData[0] & 0xF0) == 0xE0)
    { 
      trc(F("start  answer_E0"));
      answer_E0();  
    }
    
    traceInData();   
  }

}

void answer_E0(){
                
  String zlabel=" ";

  if (inData[14] != 1)
  {
    for (int k = 15; k <= 30; k++)
    {
      zlabel = zlabel + String(inData[k]);
    }
    zlabel.trim();
  }
  
  processMessage( inData[10], inData[12], zlabel);
  //if (inData[10] == 45 && inData[12] == 5) // Winload
  if (inData[10] == 4 && inData[12] == 11) // NEware
  {
    PanelConnected = false;
    trc(F("Recieved PanelConnected = false"));
    // DEBUG message over MQTT 
    //sendMQTT(root_topicOut, "{\"debug\":\"Received PanelConnected = false\"}" , false);
  }
  //else if (inData[10] == 45 && inData[12] == 4 ) // Winload
  else if (inData[10] == 4 && inData[12] == 10 ) // NEware
  {
    PanelConnected = true;
      trc(F("Recieved PanelConnected = true"));
      // DEBUG message over MQTT 
      ///sendMQTT(root_topicStatus, "{\"status\":\"Received PanelConnected = true\"}" , false);

  }
}

void blink(int duration) {
   
  digitalWrite(LED_BUILTIN,LOW);
  delay(duration);
  digitalWrite(LED_BUILTIN,HIGH);
 
}

void saveConfigCallback () {
  trc(F("Should save config"));
  shouldSaveConfig = true;
}

void callback(char* topic, byte* payload, unsigned int length) {
  // In order to republish this payload, a copy must be made
  // as the orignal payload buffer will be overwritten whilst
  // constructing the PUBLISH packet.
   if (RunningCommand){
     trc("Command already Running exiting");
      return;
    }
  trc(F("Hey I got a callback "));
  // Conversion to a printable string
  payload[length] = '\0';
//  sendMQTT(root_topicDebug,payload[length], false);
  inPayload data;
//  sendMQTT(root_topicDebug,inPayload, false);
//  sendMQTT(root_topicDebug,data, false);

  
  trc("JSON Returned! ====");
  String progEvent = String((char *)payload);
//  trc("progEvent is: "); 
//  traceInData();
//  sendMQTT(root_topicOut,progEvent, false);
  
  if (progEvent.indexOf("=")>0)
  {
    progEvent.toLowerCase();
  
    if (progEvent=="trace=1")
    {
      TRACE=1;
      Serial1.println("Trace is ON");
    }
    else if (progEvent == "trace=0")
    {
      TRACE=0;
      Serial1.println("Trace is OFF");
      
    }
    else if (progEvent == "ota=0")
    {
      OTAUpdate=0;
      Serial1.println("OTA update is OFF");
      
    }
    else if (progEvent == "ota=1")
    {
      OTAUpdate=1;
      Serial1.println("OTA update is ON");
      
    }
    else if (progEvent == "sendeventdescriptions=1")
    {
      SendEventDescriptions = 1;
      Serial1.println("SendEventDescriptions is ON");
      
    }
    else if (progEvent == "sendeventdescriptions=0")
    {
      SendEventDescriptions = 0;
      Serial1.println("SendEventDescriptions is OFF");
      
    }
    
    else 
    {
      trc(F("error in ProgEvent payload "));
      
    }
    return ;
  }
    
  trc(F("parsing Recievied Json Data"));
  data = Decodejson((char *)payload);
  if (JsonParseError)
  {
    trc(F("Error parsing Json Command") );
    ///sendMQTT(root_topicStatus, "{\"status\":\"Error parsing JSON command\"}" , false);
    JsonParseError=false;
    return;
  }
  trc(F("Json Data is ok "));
  ///sendMQTT(root_topicStatus, "{\"status\":\"JSON data is ok\"}" , false);
  PanelError = false;
    RunningCommand=true;
    PanelConnected=false; // ADDED FOR CHECKING TO SEE IF THIS IS A PROBLEM
  if (!PanelConnected)
  {
    trc(F("Panel not logged in"));
    ///sendMQTT(root_topicStatus, "{\"status\":\"Panel not logged in\"}" , false);
    doLogin(data.PcPasswordFirst2Digits, data.PcPasswordSecond2Digits);
    trc(PanelConnected?"Panel logged in":"Panel login failed");
    //sendMQTT(root_topicStatus, "{\"status\":\"Panel login failed\"}" , false);
  }
      
 
  if (!PanelConnected)
  {
    trc(F("Problem connecting to panel"));
    ///sendMQTT(root_topicStatus, "{\"status\":\"Problem connecting to panel\"}" , false);
    // DEBUG message over MQTT 
    //sendMQTT(root_topicOut, "{\"debug\":\"Problem connecting to panel\"}" , false);
  }else if (data.Command == 0x50  ) 
  {
    trc(F("Running panel status command"));
    ///sendMQTT(root_topicStatus, "{\"status\":\"Running Panel Status Command\" }", false);
    if (data.Subcommand==0)
    {
     PanelStatus0();
    }
    if (data.Subcommand==1)
    {
     PanelStatus1();
    }
  }
  else if (data.Command == 0x91  )  {
    trc(F("Running ArmState"));
    ///sendMQTT(root_topicStatus, "{\"status\":\"Running ArmState 0x91 Command\" }", false);
    ArmState();
  }
  else if (data.Command == 0x30)
  {
    trc(F("Command Setdate"));
    //panelSetDate();
  }
  
  else if (data.Command != 0x00  )  {
    trc(F("Running Command"));
    ///sendMQTT(root_topicStatus, "{\"status\":\"Running other Command\" }", false);
    ControlPanel(data);
  } 
  else  {
    trc(F("Bad Command "));
    ///sendMQTT(root_topicStatus, "{\"status\":\"Bad Command\" }", false);
  }
  
  RunningCommand=false;
  
}

byte getPanelCommand(String data){
  byte retval=0x00;

  data.toLowerCase();
  if (data == "stay" || data=="0")
  {
    retval = Stay_Arm; 
  }
  else if (data == "arm" || data=="1")
  {    
    retval= Full_Arm;
  }
  else if (data == "sleep" || data=="2")
  {
    retval= Sleep_Arm; 
  }
  else if (data == "disarm" || data == "3")
  {
   retval=Disarm;  
  }
  else if (data == "bypass" || data == "10")
  {
    retval=Bypass; 
  }
  else if (data == "pgm_on" || data == "pgmon")
  {
    retval = PGMon;
  }
  else if (data == "pgm_off" || data == "pgmoff")
  {
    retval = PGMoff;
  }
  else if (data == "panelstatus" )
  {
    retval=0x50;
  }
  else if (data == "setdate")
  {
    retval=0x30; 
  }
  else if (data == "armstate")
  {
    retval=0x91;
  }
  
    if(TRACE)
    {
      Serial1.print("returned command = ");
      Serial1.println(retval , HEX);
    }
  return retval;
}

void panelSetDate(){
  
  // dateTime = NTPch.getNTPtime(timezone, 1);
  
  // if (dateTime.valid)
  // {
    
  //   byte actualHour = dateTime.hour;
  //   byte actualMinute = dateTime.minute;
  //   byte actualyear = (dateTime.year - 2000) & 0xFF ;
  //   byte actualMonth = dateTime.month;
  //   byte actualday = dateTime.day;
  

  //   byte data[MessageLength] = {};
  //   byte checksum;
  //   memset(data, 0, sizeof(data));

  //   data[0] = 0x30;
  //   data[4] = 0x21;         //Century
  //   data[5] = actualyear;   //Year
  //   data[6] = actualMonth;  //Month
  //   data[7] = actualday;    //Day
  //   data[8] = actualHour;   //Time
  //   data[9] = actualMinute; // Minutes
  //   data[33] = 0x05;

  //   checksum = 0;
  //   for (int x = 0; x < MessageLength - 1; x++)
  //   {
  //     checksum += data[x];
  //   }

  //   data[36] = checksumCalculate(checksum);
  //   trc("sending setDate command to panel");
  //   Serial.write(data, MessageLength);
  //   readSerialQuick();
    
  // }else
  // {
  //   trc(F("ERROR getting NTP Date "));
  //   sendMQTT(root_topicStatus,"{\"status\":\"ERROR getting NTP Date  \" }", false);
  // }
}

void ControlPanel(inPayload data){
  byte armdata[MessageLength] = {};
  byte checksum;
  for (int i=0; i <= MessageLength;i++)
  {
    armdata[i]=0x00;
  }

  armdata[0] = 0x40;
  armdata[2] = data.Subcommand;
  //armdata[2] = 0x30;
  //armdata[3] = data.Subcommand;
  //armdata[4] = 0x01;
  //armdata[4] = data.Subcommand;
  //armdata[6] = data.Command;
  //armdata[33] = 0x01; // Winload Direct
  //armdata[33] = 0x05; // NEware Direct
  armdata[34] = 0x00;
  armdata[35] = 0x00;
  checksum = 0;
  for (int x = 0; x < MessageLength - 1; x++)
  {
    checksum += armdata[x];
  }
  armdata[36] = checksumCalculate(checksum);
  
  trc(F("sending Arm command to panel"));
  Serial.write(armdata, MessageLength);
  //char blah[28];
  //sprintf(blah, "%04d, %04d, %04d, %04d, %04d", armdata[0], armdata[2], armdata[3], armdata[34], armdata[35]);
  ///sendMQTT(root_topicStatus,"{\"status\":\"Sending 0x40 command to panel\" }", false);
  //sendMQTT(root_topicStatus, blah, false);
  if (data.Subcommand == 0x60)
    {
//      datachanged=true;
      hassioStatus.stringArmStatus = "disarmed";
      sendMQTT(root_topicHassioArm,hassioStatus.stringArmStatus, true);
    }
}

void PanelDisconnect(){
  byte data[MessageLength] = {};
  byte checksum;
  memset(data, 0, sizeof(data));

  data[0] = 0x70;
  data[2] = 0x05;
  data[33] = 0x05;

  checksum = 0;
  for (int x = 0; x < MessageLength - 1; x++)
  {
    checksum += data[x];
  }
  data[36] = checksumCalculate(checksum);  
  Serial.write(data, MessageLength);
  ///sendMQTT(root_topicStatus, "{\"status\":\"Sending Disconnect to panel\"}" , false);
  
}

void PanelStatus0(){
  byte data[MessageLength] = {};
  byte checksum;
  memset(data, 0, sizeof(data));

  data[0] = 0x50;
  data[1] = 0x00;
  data[2] = 0x80;
  data[3] = 0x00;
  data[33] = 0x05;
 checksum = 0;
  for (int x = 0; x < MessageLength - 1; x++)
  {
    checksum += data[x];
  }

  data[36] = checksumCalculate(checksum);
  trc(F("sending Panel Status 0 command to panel"));
  Serial.write(data, MessageLength);  


   readSerialQuick();
   bool Timer_Loss = bitRead(inData[4],7);
    bool PowerTrouble  = bitRead(inData[4],1);
    bool ACFailureTroubleIndicator = bitRead(inData[6],1);
    bool NoLowBatteryTroubleIndicator = bitRead(inData[6],0);
    bool TelephoneLineTroubleIndicator = bitRead(inData[8],0);
    int ACInputDCVoltageLevel = inData[15];
    int PowerSupplyDCVoltageLevel =inData[16];
    int BatteryDCVoltageLevel=inData[17];

    
        StaticJsonBuffer<256> jsonBuffer;
        JsonObject& root = jsonBuffer.createObject();
        root["Timer_Loss"]=String(Timer_Loss);
        root["PowerTrouble"]=String(PowerTrouble);
        root["ACFailureTrouble"]=String(ACFailureTroubleIndicator);
        root["TelephoneLineTrouble"]=String(TelephoneLineTroubleIndicator);
        root["PSUDCVoltage"]=String(PowerSupplyDCVoltageLevel);
        root["BatteryDCVoltage"]=String(BatteryDCVoltageLevel);
        root["BatteryTrouble"]=String(NoLowBatteryTroubleIndicator);
        char output[256];
        root.printTo(output);
        sendCharMQTT(root_topicOut,output ,false);  
    
    String Zonename ="";
    int zcnt = 0;
        
    for (int i = 19 ; i <= 22;i++)
    {
      
      StaticJsonBuffer<256> jsonBuffer;
        JsonObject& zonemq = jsonBuffer.createObject();
     for (int j = 0 ; j <= 8;j++) 
       {
         Zonename = "Z" + String(++zcnt);

       
        zonemq[Zonename] =  bitRead(inData[i],j);
        
        //trc (retval);
       
       }
       char Zonemq[256];
        zonemq.printTo(Zonemq);
        sendCharMQTT(root_topicOut,Zonemq,false); 
    }
    

   
}

void ArmState(){
    sendArmStatus();
}

void PanelStatus1(){
  byte data[MessageLength] = {};
  byte checksum;
  memset(data, 0, sizeof(data));


  data[0] = 0x50;
  data[1] = 0x00;
  data[2] = 0x80;
  data[3] = 0x01;
  data[33] = 0x05;

  checksum = 0;
  for (int x = 0; x < MessageLength - 1; x++)
  {
    checksum += data[x];
  }
  data[36] = checksumCalculate(checksum);
  trc(F("sending Panel Status 1 command to panel"));
  ///sendMQTT(root_topicStatus, "{\"status\":\"Sending Panel Status 1 to panel\"}" , false);
  Serial.write(data, MessageLength);
  
  readSerialQuick();

  bool Fire=bitRead(inData[17],7);
  bool Audible=bitRead(inData[17],6);
  bool Silent=bitRead(inData[17],5);
  bool AlarmFlg=bitRead(inData[17],4);
  bool StayFlg=bitRead(inData[17],2);
  bool SleepFlg=bitRead(inData[17],1);
  bool ArmFlg=bitRead(inData[17],0);

    StaticJsonBuffer<256> jsonBuffer;
    char panelst[256];
        JsonObject& panelstatus1 = jsonBuffer.createObject();
        panelstatus1["Fire"]=Fire;
        panelstatus1["Audible"]=Audible;
        panelstatus1["Silent"]=Silent;
        panelstatus1["AlarmFlg"]=AlarmFlg;
        panelstatus1["StayFlg"]=StayFlg;
        panelstatus1["SleepFlg"]=SleepFlg;
        panelstatus1["ArmFlg"]=ArmFlg;
        panelstatus1["zoneisbypassed"]=bool(bitRead(inData[18],3));
            
        panelstatus1.printTo(panelst);
        sendCharMQTT(root_topicOut,panelst,false);  

     if (AlarmFlg)
    {
       hassioStatus.stringArmStatus="triggered";
       homekitStatus.stringArmStatus="ALARM_TRIGGERED";
       homekitStatus.intArmStatus=4;
    }
    else if (StayFlg)
    {
       hassioStatus.stringArmStatus="armed_home";
       homekitStatus.stringArmStatus="STAY_ARM";
       homekitStatus.intArmStatus=0;
    }else if (SleepFlg)
    {
        hassioStatus.stringArmStatus="armed_home";
       homekitStatus.stringArmStatus="NIGHT_ARM";
       homekitStatus.intArmStatus=2;
    }
    else if (ArmFlg)
    {
        hassioStatus.stringArmStatus = "armed_away";
         homekitStatus.stringArmStatus = "AWAY_ARM";
         homekitStatus.intArmStatus = 1;
    }
    else if (!SleepFlg && !StayFlg && !ArmFlg)
    {
        hassioStatus.stringArmStatus = "disarmed";
        homekitStatus.stringArmStatus = "DISARMED";
        homekitStatus.intArmStatus = 3;
    }
    
    else
    {
        hassioStatus.stringArmStatus = "unknown";
        homekitStatus.stringArmStatus = "unknown";
        homekitStatus.intArmStatus = 99;
    }
    //sendMQTT(root_topicArmStatus,retval);
    sendArmStatus();
}

void readSerialQuick(){
 while (Serial.available()<37  )  
  { 
    yield(); 
  }                            
  {
    
    trc("Reading readSerialQuick");
    // DEBUG message over MQTT 
    //sendMQTT(root_topicOut, "{\"debug\":\"Reading readSerialQuick\"}" , false);
    
      pindex=0;
  
      while(pindex < 37) // Paradox packet is 37 bytes 
      {
          inData[pindex++]=Serial.read();  
      } 
      inData[++pindex]=0x00; // Make it print-friendly
      trc("readSerialQuick data is");
      traceInData();
  }
}

void doLogin(byte pass1, byte pass2){
  //byte data0[MessageLength] = {};
  byte data[MessageLength] = {};
  byte data1[MessageLength] = {};
  byte checksum;

  trc(F("Running doLogin Function"));
  ///sendMQTT(root_topicStatus, "{\"status\":\"Running doLogin function\"}" , false);
  // DEBUG message over MQTT 
  //sendMQTT(root_topicOut, "{\"debug\":\"Running doLogin Function\"}" , false);

/*
for (int i=0; i <= MessageLength;i++)
  {
    data0[i]=0x00;
    data[i]=0x00;
    data1[i]=0x00;
  }
  //memset(data1, 0, sizeof(data1));

    data[0] = 0x72;

  checksum = 0;
  for (int x = 0; x < MessageLength - 1; x++)
  {
    checksum += data[x];
  }
  data[36] = checksumCalculate(checksum);
   trc(F("sending command 0x5f to panel"));
   sendMQTT(root_topicStatus, "{\"status\":\"sending command 0x5f to panel\"}" , false);


    Serial.write(data, MessageLength);
   
    readSerialQuick();

    
    trc(F("got response from 0x72 command"));
    sendMQTT(root_topicStatus, "{\"status\":\"got response from 0x72 command\"}" , false);
    
// NEXT
/* Commented out for 72 test
 for (int i=0; i <= MessageLength;i++)
  {
    data[i]=0x00;
    data1[i]=0x00;
  }
  //memset(data1, 0, sizeof(data1));
*/
  
  data[0] = 0x5f;
  data[1] = 0x20;
  data[33] = 0x00;
  data[34] = 0x00;
  data[36] = 0x7f;
/*
  checksum = 0;
  for (int x = 0; x < MessageLength - 1; x++)
  {
    checksum += data[x];
  }
  data[36] = checksumCalculate(checksum);*/
   trc(F("sending command 0x5f to panel"));
   ///sendMQTT(root_topicStatus, "{\"status\":\"sending command 0x5f to panel\"}" , false);


//  char readymsg[64];
//  sprintf(readymsg, "{\"firmware\":\"SYSTEM %s\"}", firmware);
//  sendCharMQTT(root_topicStatus,readymsg,false);

   
   // DEBUG message over MQTT 
   //sendMQTT(root_topicOut, "{\"debug\":\"sending command 0x5f to panel\"}" , false);
   
    Serial.write(data, MessageLength);
   
    readSerialQuick();

    
    trc(F("got callback from 0x5f command"));
    ///sendMQTT(root_topicStatus, "{\"status\":\"Got callback from 0x5f command\"}" , false);
    // DEBUG message over MQTT 
    //sendMQTT(root_topicOut, "{\"debug\":\"got callback from 0x5f command\"}" , false);
      data1[0] = 0x00;
      data1[1] = 0x00;
      data1[2] = inData[2];
      data1[3] = inData[3];
      data1[4] = inData[4];
      data1[5] = inData[5];
      data1[6] = inData[6];
      data1[7] = inData[7];
      data1[7] = inData[8];
      data1[9] = inData[9];
      data1[10] = inData[10];
      data1[11] = inData[11];
      data1[12] = inData[12];
      data1[13] = inData[13];// 0x55; // NEware 0x55, Winload = 0x0xx
      data1[14] = inData[14];
      data1[15] = inData[15];
      data1[16] = inData[16];
      data1[17] = inData[17];
      data1[18] = inData[18];
      data1[19] = inData[19];
      data1[20] = inData[20];
      data1[21] = inData[21];
      data1[22] = inData[22];
      data1[23] = inData[23];
      data1[24] = inData[24];
      //data1[33] = 0x05;

      checksum = 0;
      for (int x = 0; x < MessageLength - 1; x++)
      {
        checksum += data1[x];
      }
      data1[36] = checksumCalculate(checksum);
      
      trc("sending command 0x00 to panel");
      ///sendMQTT(root_topicStatus, "{\"status\":\"Sending 0x00 resposne to panel to initialize\"}" , false);

      Serial.write(data1, MessageLength);  
           
      readSerialQuick();
      if ((inData[0] & 0xF0) == 0x10)
      {
        PanelConnected = true;
        ///sendMQTT(root_topicStatus, "{\"status\":\"Panel Connected\"}" , false);
      }
      
      
      trc(F("Panel login complete"));
      ///sendMQTT(root_topicStatus, "{\"status\":\"Panel dologin complete\"}" , false);
}

struct inPayload Decodejson(char *Payload){
  inPayload indata;
  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.parseObject(Payload);
  if (!root.success())
  {
    indata = {0x00,0x00,0x00,0x00};
    trc("JSON parsing failed!");
    ///sendMQTT(root_topicStatus, "{\"status\":\"JSON parsing failed\"}" , false);
    // DEBUG message over MQTT 
    //sendMQTT(root_topicOut, "{\"debug\":\"JSON parsing failed\"}" , false);
    JsonParseError=true;
    return indata;
  }
  else
  {
    char charpass1[4];
    char charpass2[4];
    char charsubcommand[4];
    
    String password = root["password"];
    String command = root["Command"];
    String subcommand = root["Subcommand"];

    String pass1 = password.substring(0, 2);
    String pass2 = password.substring(2, 4);

    // trc(pass1);
    // trc(pass2);

    pass1.toCharArray(charpass1, 4);
    pass2.toCharArray(charpass2, 4);
    subcommand.toCharArray(charsubcommand,4);

    unsigned long number1 = strtoul(charpass1, nullptr, 16);
    unsigned long number2 = strtoul(charpass2, nullptr, 16);
    unsigned long number3 = strtoul(charsubcommand, nullptr, 16);

    if (number2 < 10)
      number2 = number2 + 160;

    if (number1 < 10)
      number1 = number1 + 160;


    byte PanelPassword1 = number1 & 0xFF; 
    byte PanelPassword2 = number2 & 0xFF; 
    byte SubCommand = number3 & 0xFF;

    byte CommandB = getPanelCommand(command) ;
  
    inPayload data1 = {PanelPassword1, PanelPassword2, CommandB, SubCommand};
    ///sendMQTT(root_topicStatus, "{\"status\":\"JSON parsing complete\"}" , false);
    return data1;
  }
  return indata;
}

void serial_flush_buffer(){
  trc("starting serial flush");
  Serial.flush();
  delay(1000);
  
  while (Serial.read() >= 0)
  {
    trc("flushing........");
  }
  trc("serial clean");
}

void setup_wifi(){
  
    WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
    WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqtt_port, 6);

    WiFiManager wifiManager;
    if (ResetConfig)
    {
      trc(F("Resetting wifiManager"));
      WiFi.disconnect();
      wifiManager.resetSettings();
    }
       
    if (mqtt_server=="" || mqtt_port=="")
    {
      trc(F("Resetting wifiManager"));
      WiFi.disconnect();
      wifiManager.resetSettings();
      ESP.reset();
      delay(1000);
    }
    else
    {
      trc(F("values ar no null "));
    }


    wifiManager.setSaveConfigCallback(saveConfigCallback);
    wifiManager.setConfigPortalTimeout(180);
    
    wifiManager.addParameter(&custom_mqtt_server);
    wifiManager.addParameter(&custom_mqtt_port);
        
    if (!wifiManager.autoConnect(Hostname, "")) {
      trc(F("failed to connect and hit timeout"));
      delay(3000);
      //reset and try again, or maybe put it to deep sleep
      ESP.reset();
      delay(5000);
    }
    //if you get here you have connected to the WiFi
    trc(F("connected...yeey :)"));
  
    strcpy(mqtt_server, custom_mqtt_server.getValue());
    strcpy(mqtt_port, custom_mqtt_port.getValue());
    
    //save the custom parameters to FS
    if (shouldSaveConfig) {
      trc(F("saving config"));
      DynamicJsonBuffer jsonBuffer;
      JsonObject& json = jsonBuffer.createObject();
      json["mqtt_server"] = mqtt_server;
      json["mqtt_port"] = mqtt_port;
      
      File configFile = SPIFFS.open("/config.json", "w");
      if (!configFile) {
        trc(F("failed to open config file for writing"));
      }
  
      json.printTo(Serial1);
      json.printTo(configFile);
      configFile.close();
      //end save
    }
  
    trc(F("Setting Mqtt Server values"));
    trc(F("mqtt_server : "));
    trc(mqtt_server);
    trc(F("mqtt_server_port : "));
    trc(mqtt_port);

    trc(F("Setting Mqtt Server connection"));
    unsigned int mqtt_port_x = atoi (mqtt_port); 
    client.setServer(mqtt_server, mqtt_port_x);
    
    client.setCallback(callback);
   
    reconnect();
    
    
    trc(F("WiFi connected"));
    Serial1.print("IP address:");
    Serial1.println(WiFi.localIP());
  
}

boolean reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    trc("Attempting MQTT connection...");
    String mqname =  WiFi.macAddress();
    char charBuf[50];
    mqname.toCharArray(charBuf, 50) ;

//    if (client.connect(charBuf,mqtt_user,mqtt_password,root_topicStatus,0,true,"{\"status\":\"offline\"}")) { // original
    if (client.connect(charBuf,mqtt_user,mqtt_password,root_topicStatus,0,true,"offline")) {
    // Once connected, publish an announcement...
      //client.publish(root_topicOut,"connected");
      trc("MQTT connected");
      //sendMQTT(root_topicStatus, "{\"status\":\"Paradox connected\"}", false); // original
      sendMQTT(root_topicStatus, "online", true);
      //Topic subscribed so as to get data
      String topicNameRec = root_topicIn;
      //Subscribing to topic(s)
      subscribing(topicNameRec);
    } else {
      trc("failed, rc=");
      trc(String(client.state()));
      trc(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
  return client.connected();
}

void handleMqttKeepAlive(){

  if (!client.connected()) {
    long now = millis();
    if (now - lastReconnectAttempt > 5000) {
      lastReconnectAttempt = now;
      // Attempt to reconnect
      if (reconnect()) {
        lastReconnectAttempt = 0;
      }
    }
  } else {
    // Client connected
    client.loop();
  }
}

void subscribing(String topicNameRec){ // MQTT subscribing to topic
  char topicStrRec[26];
  topicNameRec.toCharArray(topicStrRec,26);
  // subscription to topic for receiving data
  boolean pubresult = client.subscribe(topicStrRec);
  if (pubresult) {
    trc("subscription OK to");
    trc(topicNameRec);
  }
}

void mountfs(){
   if (SPIFFS.begin()) {
    trc(F("mounted file system"));
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      trc(F("reading config file"));
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        trc(F("opened config file"));
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial1);
        if (json.success()) {
          trc(F("\nparsed json"));

          strcpy(mqtt_server, json["mqtt_server"]);
          strcpy(mqtt_port, json["mqtt_port"]);
          
        } else {
          trc(F("failed to load json config"));
          
        }
      }
    }
    else
    {
      trc(F("File /config.json doesnt exist"));
      //SPIFFS.format();
      trc(F("Formatted Spiffs"));    
    }
  } else {
    trc(F("failed to mount FS"));
  }
}

void trc(String msg){
  if (TRACE) {
  Serial1.println(msg);
  //sendMQTT(root_topicOut,msg, false);

  }
}

void traceInData(){
  if (TRACE && (inData[0] & 0xF0) != 0xE0)
  {
    
      Serial1.print("Address-");
      Serial1.print("0");
      Serial1.print("=");
      Serial1.println(inData[0], HEX);
    
  }
}



 
