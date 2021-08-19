#include <Arduino.h>
#include <ESP8266WiFi.h>  // wifi board
#include <PubSubClient.h> // MQTT
#include "Credential.h"   // id wifi + mqtt

#define PIN_RELAY 0 // relay board linked on GPIO0
#define ESP_NAME "ESPRelay"
#define TOPIC_RELAY "sdb/radiator"

WiFiClient espClient;
PubSubClient clientMQTT(mqttServer, mqttPort, espClient);
char message_buff[100];

void blinkLED(int nb)
{
  for (int i = 0; i < nb; i++)
  {
    digitalWrite(LED_BUILTIN, LOW);
    delay(60);
    digitalWrite(LED_BUILTIN, HIGH);

    if (i < nb - 1)
      delay(100);
  }
}

void connectWifi()
{
  // wifi
  if (WiFi.status() != WL_CONNECTED)
  {
    WiFi.begin(ssid, password);

    if (WiFi.waitForConnectResult() != WL_CONNECTED)
      ;

    blinkLED(2);
  }
  delay(100);
  // mqtt
  if (!clientMQTT.connected())
  {
    if (clientMQTT.connect(ESP_NAME, mqttUser, mqttPassword))
    {
      clientMQTT.subscribe(TOPIC_RELAY);
    }
    else
    {
      delay(5e3);
    }
  }
}

void callback(char *topic, byte *payload, unsigned int length)
{
  int i;
  for (i = 0; i < length; i++)
  {
    message_buff[i] = payload[i];
  }
  message_buff[i] = '\0'; 

  String msgString = String(message_buff); 
  uint8_t msgVal = msgString.toInt();      
  if (strcmp(topic, TOPIC_RELAY) == 0)
  {                                 // Returns 0 if the strings are equal, so we have received our topic.
    if (msgVal == 1)                // stop
    {                               // Payload of our topic is 0, so we'll turn off the store.
      digitalWrite(PIN_RELAY, LOW); // make PIN_RELAY output low
    }
    else if (msgVal == 0)            // start
    {                                // Payload of our topic is 1, so we'll turn on the store.
      digitalWrite(PIN_RELAY, HIGH); // make PIN_RELAY output high
    }
  }
}

void setup()
{
  Serial.begin(115200);
  delay(100);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PIN_RELAY, OUTPUT);
  delay(100);
  // init wifi
  WiFi.mode(WIFI_STA);
  WiFi.setHostname(ESP_NAME);
  // init MQTT
  clientMQTT.setServer(mqttServer, mqttPort);
  clientMQTT.setCallback(callback);
  delay(100);
  clientMQTT.publish(TOPIC_RELAY, "0"); // Relay start off
  blinkLED(5);
}

void loop()
{
  if (!clientMQTT.connected())
  {
    connectWifi();
  }
  clientMQTT.loop();
}
