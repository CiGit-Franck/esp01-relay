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

void connectWifi()
{
  // wifi
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.print("Connecting to ");
    Serial.println(ssid);
    Serial.println("...");
    WiFi.begin(ssid, password);

    if (WiFi.waitForConnectResult() != WL_CONNECTED)
      ;

    Serial.println("WiFi connected.");
  }
  delay(100);
  // mqtt
  if (!clientMQTT.connected())
  {
    Serial.println("Connecting to MQTT...");
    if (clientMQTT.connect(ESP_NAME, mqttUser, mqttPassword))
    {
      Serial.println("connected");
      clientMQTT.subscribe(TOPIC_RELAY);
    }
    else
    {
      Serial.print("failed with state ");
      Serial.println(clientMQTT.state());
      delay(5e3);
    }
    clientMQTT.publish(TOPIC_RELAY, "0");
  }
}

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived: [");
  Serial.print(topic);
  Serial.println("]"); // Prints out any topic that has arrived and is a topic that we subscribed to.

  int i;
  for (i = 0; i < length; i++)
  {
    message_buff[i] = payload[i];
  }
  message_buff[i] = '\0'; // We copy payload to message_buff because we can't make a string out of a byte based payload.

  String msgString = String(message_buff); // Converting our payload to a string so we can compare it.
  uint8_t msgVal = msgString.toInt();      // Now, let's convert that payload to an integer for numeric comparisons below.
  // If we just used strings as the payload, then we don't need to convert to int.
  // Also, if we use JSON data as the payload, then we need to add that functionality as well.
  Serial.print("Message: ");
  Serial.println(msgVal);
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

  Serial.println("=== Begin setup ===");
  pinMode(LED_BUILTIN, OUTPUT); // Initialize the LED_BUILTIN pin as an output
  pinMode(PIN_RELAY, OUTPUT);
  delay(100);
  // init wifi
  WiFi.mode(WIFI_STA);
  WiFi.setHostname(ESP_NAME);
  clientMQTT.setServer(mqttServer, mqttPort);
  clientMQTT.setCallback(callback);
}

void loop()
{
  Serial.println("=== Begin loop ===");
  if (!clientMQTT.connected())
  {
    connectWifi();
  }
  clientMQTT.loop();
}
