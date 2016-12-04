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

#define CRITICAL_LEVEL_OF_CO_IN_PPM 1000

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress insideThermometer;

byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xEA };
IPAddress ip(192, 168, 1, 15);
IPAddress server(192, 168, 1, 123);

EthernetClient ethClient;
PubSubClient client(ethClient);

unsigned long time;

float temperatureInCel = 0;
int coLevelInPpm = 0;
int naturalGasLevelInPpm = 0;

void reconnect() {
  Serial.println("try connect");
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("arduinoClient")) {
      client.setCallback(callback);
      client.subscribe("commands");
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
  if (!client.connected()) {
    delay(1000);
    reconnect();
  }
  if (millis() > (time + 5000)) {
    time = millis();

    float temperature = 0;
    String sensorTemperatureDataJson = "";
    if (abs((temperature = getTemperature()) - temperatureInCel) > 0.01){
      temperatureInCel = temperature;
      sensorTemperatureDataJson = "{\"sensorType\": \"temperature\", \"sensorDataValue\":"  + String(temperatureInCel) + ",\"unit\":\"celsium\"}";
      publishMessageToMQTTBrocker(sensorTemperatureDataJson);
      Serial.println(sensorTemperatureDataJson);
    }

    int co = 0;
    String sensorCODataJson = "";
    if ((co = getCOLevel()) != coLevelInPpm){
      coLevelInPpm = co;
      checkCOLevel(coLevelInPpm);
      sensorCODataJson = "{\"sensorType\": \"co\", \"sensorDataValue\":"  + String(coLevelInPpm) + ",\"unit\":\"ppm\"}";
      publishMessageToMQTTBrocker(sensorCODataJson);
      Serial.println(sensorCODataJson);
    }

    int naturalGas = 0;
    String sensorNaturalGasDataJson = "";
    if ((naturalGas = getNaturalGasConcentrationLevel()) != naturalGasLevelInPpm){
      naturalGasLevelInPpm = naturalGas;
      sensorNaturalGasDataJson = "{\"sensorType\": \"gas\", \"sensorDataValue\":"  + String(naturalGasLevelInPpm) + ",\"unit\":\"ppm\"}";
      publishMessageToMQTTBrocker(sensorNaturalGasDataJson);
      Serial.println(sensorNaturalGasDataJson);
    }
  }

  client.loop();
}

void publishMessageToMQTTBrocker(String dataToBrocker) {
  char message_buff[255];
  dataToBrocker.toCharArray(message_buff, dataToBrocker.length()+1);
  client.publish("/sensors", message_buff);
}

void callback(char* topic, byte* payload, unsigned int length) {
  //TBD
}

String buildSensorDataJsonMessage(String temperature, String co, String naturalGas){
      return "[" + String(temperature) + "," + String(co) + "," + String(naturalGas) + "]";
}

void checkCOLevel(float coLevelInPpm) {
  if (coLevelInPpm > CRITICAL_LEVEL_OF_CO_IN_PPM){
      beep(50);
      beep(200);
      beep(50); 
    }
}

int getCOLevel(){
    int mq7_value = analogRead(MQ_7_AOUT_PIN);
    return mq7_value;
}

int getNaturalGasConcentrationLevel(){
  int mq5_value= analogRead(MQ_5_AOUT_PIN);
  return mq5_value;
}

float getTemperature() {
  sensors.requestTemperatures();
  float tempC = sensors.getTempC(insideThermometer);
  return tempC;
}

void beep(int t) {
  pinMode(BUZZER_OUTPUT, OUTPUT);
  tone(BUZZER_OUTPUT, t);
  delay(200);
  noTone(BUZZER_OUTPUT);
  pinMode(BUZZER_OUTPUT, INPUT);
}

