#include <ESP8266WiFi.h>
//#include <WiFiClient.h>
#include <PubSubClient.h>


#ifndef STASSID
#define STASSID "HS51"
#define STAPSK "VaeHS513210V"
#endif

const char* mqtt_server = "192.168.1.245";

const char* ssid = STASSID;
const char* password = STAPSK;


WiFiClient espClient;
PubSubClient client(espClient);


String myMac = "";


const short int pins[] = {0,1,3,4,5,12,13,14};
short int pinsMode[] = {0,0,0,0,0,0,0,0};
unsigned long int pinsPreviosMilli[] = {0,0,0,0,0,0,0,0};
int pinsIntervalA[]= {0,0,0,0,0,0,0,0};
int pinsIntervalB[]= {0,0,0,0,0,0,0,0};






void callback(char* topic, byte* payload, unsigned int length) {

String testmymac;
for(int i=0; i <12; i++){
  testmymac = testmymac + (char)payload[i];
  }
if((myMac == testmymac) && (length >= 23)){
  int thispin =  (char)payload[13] - '0';
  
  if((char)payload[14]=='0'){
    pinMode(pins[thispin],INPUT);
    pinsMode[thispin]= 1;

    int timeOnINT = 0;
    timeOnINT = (((char)payload[15]) - '0') * 10000;
    timeOnINT += (((char)payload[16]) - '0') * 1000;
    timeOnINT += (((char)payload[17]) - '0') * 100;
    timeOnINT += (((char)payload[18]) - '0') * 10;
    timeOnINT += (((char)payload[19]) - '0');
    pinsIntervalA[thispin] = timeOnINT * 1000;

    timeOnINT = 0;
    timeOnINT = (((char)payload[20]) - '0') * 10000;
    timeOnINT += (((char)payload[21]) - '0') * 1000;
    timeOnINT += (((char)payload[22]) - '0') * 100;
    timeOnINT += (((char)payload[23]) - '0') * 10;
    timeOnINT += (((char)payload[24]) - '0');
    pinsIntervalB[thispin] = timeOnINT * 1000;
    
    }
    
  if((char)payload[14]=='1'){
    pinMode(pins[thispin],OUTPUT);
    digitalWrite(pins[thispin],HIGH);
    pinsMode[thispin]= 0;

    int timeOnINT = 0;
    timeOnINT = (((char)payload[15]) - '0') * 10000;
    timeOnINT += (((char)payload[16]) - '0') * 1000;
    timeOnINT += (((char)payload[17]) - '0') * 100;
    timeOnINT += (((char)payload[18]) - '0') * 10;
    timeOnINT += (((char)payload[19]) - '0');
    pinsIntervalA[thispin] = timeOnINT * 1000;

    timeOnINT = 0;
    timeOnINT = (((char)payload[20]) - '0') * 10000;
    timeOnINT += (((char)payload[21]) - '0') * 1000;
    timeOnINT += (((char)payload[22]) - '0') * 100;
    timeOnINT += (((char)payload[23]) - '0') * 10;
    timeOnINT += (((char)payload[24]) - '0');
    pinsIntervalB[thispin] = timeOnINT * 1000;

    pinsPreviosMilli[thispin] = millis();
    client.publish("ledstatus","ok");
    }
  }
}




// Keeps ligth on for xxx millis
void keepLigthOn(void) { 
  unsigned long currentMillis = millis();
  for(int i=0; i < 8 ;i++){
    if(!pinsMode[i]) {
      if((currentMillis - pinsPreviosMilli[i] >= pinsIntervalA[i]) && pinsIntervalA[i] != 0) {
        digitalWrite(pins[i], LOW);
        pinsIntervalA[i] = 0;
      }
    }
  }
}






void reconnect() {
 // Loop until we're reconnected
 while (!client.connected()) {
 Serial.print("Attempting MQTT connection...");
 // Attempt to connect

 char mynameis[13];
 myMac.toCharArray(mynameis,myMac.length()+1);

 if (client.connect(mynameis)) {
  Serial.println("connected");
  // ... and subscribe to topic
  client.subscribe("ledStatus");
 } else {
  Serial.print("failed, rc=");
  Serial.print(client.state());
  Serial.println(" try again in 5 seconds");
  // Wait 5 seconds before retrying
  delay(5000);
  }
 }
}



void setup(void) {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
delay(1000);
 client.setServer(mqtt_server, 1883);
 client.setCallback(callback);

  byte mac[6];
  WiFi.macAddress(mac);
  for(int i = 0 ; i< 6;i++){
    myMac=myMac + String(mac[i],HEX); 
    }

    for(int i=0; i < 8 ;i++){
      pinMode(pins[i], OUTPUT);
      digitalWrite(pins[i], LOW);
    }
}



//
int delayAliveMsg = 30000;
int lastAliveTime=0;
//
void alive(void) {
  unsigned long currentMillis = millis();
  if((currentMillis - lastAliveTime >= delayAliveMsg) || lastAliveTime == 0 ) {
    char msg[50];
    myMac.toCharArray(msg,myMac.length()+1);
    client.publish("ledUpdate",msg); 
    Serial.println("My mac is " + myMac);
    lastAliveTime = currentMillis;
  }
}

//
//
void testpininput(){
  for(int i=0; i < 8 ;i++){
    if(pinsMode[i]){
      unsigned long currentMillis = millis();
      if(currentMillis - pinsPreviosMilli[i] >= pinsIntervalA[i]) {
        if (digitalRead(pins[i])==HIGH){
          char msg[50];
          String testtest = String(myMac) + ":" + String(i);
          testtest.toCharArray(msg,testtest.length()+1);
          client.publish("ledUpdate",msg);
          Serial.println(msg);
          pinsPreviosMilli[i]=currentMillis;
          } 
        }
    }
  }
}



void loop(void) {
if (!client.connected()) {
  reconnect();
  }
client.loop();
alive();
keepLigthOn();
testpininput();
}
