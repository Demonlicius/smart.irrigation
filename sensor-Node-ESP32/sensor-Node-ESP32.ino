#include <esp_now.h>
#include <WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "DHTesp.h"

// Pines
#define ONE_WIRE_BUS 27
#define SENSOR_HUMEDAD 32
int pinDHT = 25;

// Instancias
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DHTesp dht;

// Dirección MAC del receptor
uint8_t broadcastAddress[] = {0x78, 0x42, 0x1c, 0x67, 0xAA, 0x2C};

// Estructura
typedef struct struct_message {
  float temperatura;
  float temperaturaA;
  float humedadA;
  int humedad;
  uint8_t idNodo;
} struct_message;

struct_message datosSensor;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Envío: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "OK" : "FAIL");
}

// Inicialización ESP-NOW
void initEspNow() {
  WiFi.mode(WIFI_STA); 
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error init ESP-NOW");
    return;
  }
  esp_now_register_send_cb(OnDataSent);
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);
}

void setup() {
  Serial.begin(9600);
  sensors.begin();
  dht.setup(pinDHT, DHTesp::DHT11);
  datosSensor.idNodo = 2; //cambiar id en cada dispositivo nuevo
  initEspNow();
}

void loop() {


  int lectura = analogRead(SENSOR_HUMEDAD);  
  int humedads = map(lectura, 1200, 4095, 100, 0); 

  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);

  TempAndHumidity data = dht.getTempAndHumidity();

  datosSensor.temperatura = tempC;
  datosSensor.temperaturaA = data.temperature;
  datosSensor.humedadA = data.humidity;
  datosSensor.humedad = humedads;

  delay(100); 

  esp_now_send(broadcastAddress, (uint8_t*)&datosSensor, sizeof(datosSensor));

  delay(2000); 
}
