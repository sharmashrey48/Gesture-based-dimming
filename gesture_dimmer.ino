#include <ESP8266WiFi.h>
#include <PubSubClient.h>  //callback    //reconnect 
#include <TaskScheduler.h>
#include "DHT.h"


#define DHTPIN D3
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);


void temp();
Task T1(10000, TASK_FOREVER, &temp);

Scheduler runner;


int pstate1 = LOW;
const int trigPin1 = D1;
const int echoPin1 = D2;
const char* ssid = "Does it matter?";          // SSID for your wifi Router
const char* passkey = "qwertyuiop";  //Password for wifi Router

const char* mqtt_server = "192.168.2.17" ; //mqtt server domain or IP
const char* topic = "livingroom";// topic for mqtt
const char* topic1 = "fandimmer" ;// topic for fan dimmer

//const char* username = "shrey";   //username for mqtt broker
//const char* password = "qwertyuiop";  // password for mqtt broker

// defines variables
long duration1;
int distance1;


WiFiClient espClient;
PubSubClient client(espClient);


void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, passkey);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  String topicstr = topic;
  char sub[1000];
  int c = 0;
  Serial.print("Message arrived [");        // Message Arrived [test] a
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  
    if ((char)payload[0] == 'b') {
    digitalWrite(D4, LOW);
    Serial.println("D4 is OFF");
  } if ((char)payload[0] == 'a') {
    digitalWrite(D4, HIGH);
    Serial.println("D4 is ON");
  } if ((char)payload[0] == 'c') {
    digitalWrite(D6, LOW);
  } if ((char)payload[0] == 'd') {
    digitalWrite(D6, HIGH);
  } if ((char)payload[0] == 'e') {
    digitalWrite(D7, LOW);
  } if ((char)payload[0] == 'f') {
    digitalWrite(D7, HIGH);
  } if ((char)payload[0] == 'g') {
    digitalWrite(D3, LOW);
  } if ((char)payload[0] == 'h') {
    digitalWrite(D3, HIGH);
  }
//===================================================fandimmer topic=================================================  
if (strcmp(topic,"fandimmer")==0){
   if ((char)payload[0] == '0') {
    digitalWrite(D4, LOW);
    client.publish("publish","b",true);
  }
  if ((char)payload[0] == '1') {
    analogWrite(D4, 40);
  }
  if ((char)payload[0] == '2') {
    analogWrite(D4, 50);
  }
  if ((char)payload[0] == '3') {
    analogWrite(D4, 60);
  }
  if ((char)payload[0] == '4') {
    analogWrite(D4, 80);
  }
  if ((char)payload[0] == '5') {
    digitalWrite(D4, HIGH);
    client.publish("publish", "a",true);
  }
 
}
}
//==========================================================reconnect function=================================================================
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "Nodemcu";
    // Attempt to connect
    if (client.connect(clientId.c_str())) { // will message
      Serial.println("connected");
      client.publish("notifications", "Home is online", true);
      client.subscribe(topic);
      client.subscribe(topic1);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
//=============================================================================setup===========================================================
void setup() {
  pinMode(D4, OUTPUT);
  pinMode(D6, OUTPUT);
  pinMode(D7, INPUT);
  pinMode(D5, INPUT);
  pinMode(D3, OUTPUT);
  pinMode(trigPin1, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin1, INPUT); // Sets the echoPin as an Input
  //pinMode(D0, INPUT);
  Serial.begin(115200);

  runner.init();
  Serial.println("Initialized scheduler");
  runner.addTask(T1);
  Serial.println("added T1");
  T1.enable();
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  dht.begin();
}

void loop() {
  runner.execute();
  pstate1 = readdata(D5, D4, pstate1);
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  gesture();

}
////======================================read state of GPIO input pin ==================================================
int readdata(uint8_t Rpin, uint8_t Epin , int pstate) {
  int cstate = digitalRead(Rpin);
  if (pstate != cstate) {
    if (cstate == HIGH) {

      digitalWrite(Epin, HIGH);
      client.publish("publish", "a", true);
      Serial.println("a");
    }
    else if (cstate == LOW) {

      digitalWrite(Epin, LOW);
      client.publish("publish", "b", true);
      Serial.println("b");
    }
  } return cstate;
}
//=====================================================gesture===================================================

void gesture() {
  // Clears the trigPin
  digitalWrite(trigPin1, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin1, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin1, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration1 = pulseIn(echoPin1, HIGH);
  // Calculating the distance
  distance1 = duration1 * 0.034 / 2;

//  char cstr[5];
//  itoa(distance1, cstr, 10);

  Serial.println(distance1);

  if (distance1 <= 4) {
    client.publish("fandimmer", "0", true);
    delay(100);
  }
  if (distance1 > 5  && distance1 <= 9) {
    client.publish("fandimmer", "1", true);
    delay(50);
  }
  if (distance1 > 10  && distance1 <= 13) {
    client.publish("fandimmer", "2", true);
    delay(50);
  }
  if (distance1 > 14  && distance1 <= 17) {
    client.publish("fandimmer", "3", true);
    delay(50);
  }
  if (distance1 > 18  && distance1 <= 22) {
    client.publish("fandimmer", "4", true);
    delay(50);
  }
  if (distance1 > 23  && distance1 <= 30) {
    client.publish("fandimmer", "5", true);
    delay(50);
  }
  delay(500);
}

//===================================================================Temperature And Humidity======================================================
void temp() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float f = dht.readTemperature(true);
  String temp = String(t);
  String hum = String(h);
  Serial.print("Temperature is");
  Serial.println(t);
  client.publish("temperature", temp.c_str(), true);
  client.publish("humidity", hum.c_str(), true);
  

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
}
