#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 2
#define BUZZER_OUTPUT 3

#define MQ_7_AOUT_PIN 0
#define MQ_7_DOUT_PIN 4
#define MQ_5_AOUT_PIN 1
#define MQ_5_DOUT_PIN 5

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress insideThermometer;

byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xEA };
IPAddress ip(192, 168, 1, 15);
IPAddress server(192, 168, 1, 123);

EthernetClient ethClient;
PubSubClient client(ethClient);

unsigned long time;
char message_buff[255];

void reconnect() {
  Serial.println("try connect");
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("arduinoClient")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(1000);
    }
  }
}

void setup()
{
  Serial.begin(9600);
  client.setServer(server, 1883);
  Ethernet.begin(mac, ip);
  Serial.println("Network initialized");

  Serial.print("Locating devices...");
  sensors.begin();
  Serial.print("Found ");
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" devices.");
  if (!sensors.getAddress(insideThermometer, 0)) {
    Serial.println("Unable to find address for Device 0");
  }
  sensors.setResolution(insideThermometer, 9);

  pinMode(MQ_7_DOUT_PIN, INPUT);
  pinMode(MQ_5_DOUT_PIN, INPUT);
}

void loop()
{
  /*if (!client.connected()) {
    delay(1000);
    reconnect();
    }*/
  if (millis() > (time + 5000)) {
    time = millis();
    float temperature = getTemperature();
    String pubString = "{\"sensorType\": \"temperature\", \"sensorDataValue\":"  + String(temperature) + ",\"unit\":\"celsium\"}";
    //pubString.toCharArray(message_buff, pubString.length()+1);
    Serial.println(pubString);
    //client.publish("/sensors", message_buff);

    if (abs(temperature - 20.0) > 2.0){
      //beep(50);
      //beep(200);
      //beep(50); 
    }

    //mq-7
    int mq7_limit;
    int mq7_value;
    mq7_value= analogRead(MQ_7_AOUT_PIN);
    mq7_limit= digitalRead(MQ_7_DOUT_PIN);
    Serial.print("CO value: ");
    Serial.println(mq7_value);
    Serial.print("Limit: ");
    Serial.print(mq7_limit);

    //mq-5
    int mq5_limit;
    int mq5_value;
    mq5_value= analogRead(MQ_5_AOUT_PIN);
    mq5_limit= digitalRead(MQ_5_DOUT_PIN);
    Serial.print("propane value: ");
    Serial.println(mq5_value);
    Serial.print("Limit: ");
    Serial.print(mq5_limit);

  }
  //Serial.println("online");
  //client.loop();
}

float getTemperature() {
  Serial.print("Requesting temperatures...");
  sensors.requestTemperatures();
  Serial.println("DONE");
  float tempC = sensors.getTempC(insideThermometer);
  Serial.print("Temp C: ");
  Serial.print(tempC);
  Serial.print(" Temp F: ");
  Serial.println(DallasTemperature::toFahrenheit(tempC));
  return tempC;
}

void beep(int t) {
  pinMode(BUZZER_OUTPUT, OUTPUT);
  tone(BUZZER_OUTPUT, t);
  delay(200);
  noTone(BUZZER_OUTPUT);
  pinMode(BUZZER_OUTPUT, INPUT);
}

