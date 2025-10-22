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

// Estructura mensaje de datos
typedef struct struct_message {
  float temperatura;
  float temperaturaA;
  float humedadA;
  int humedad;
  uint8_t idNodo;
} struct_message;

struct_message datosSensor;

// Estructura ACK
typedef struct struct_ack {
  uint8_t idNodo;
  bool recibido;
} struct_ack;

struct_ack ackMsg;
volatile bool ackRecibido = false;

// Callback recepción de ACK
void OnDataRecv(const esp_now_recv_info_t *recv_info, const uint8_t *incomingData, int len) {
  memcpy(&ackMsg, incomingData, sizeof(ackMsg));
  if (ackMsg.recibido && ackMsg.idNodo == datosSensor.idNodo) {
    ackRecibido = true;
  }
}


// Callback de envío
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
  esp_now_register_recv_cb(OnDataRecv);

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
  datosSensor.idNodo = 2; // Cambiar en cada nodo
  initEspNow();
}
                                                                  
void loop() {
  int lectura = analogRead(SENSOR_HUMEDAD);  
  int humedads = map(lectura, 0, 4095, 100, 0); 

  sensors.requestTemperatures();

  float tempC = sensors.getTempCByIndex(0);
  
  TempAndHumidity data = dht.getTempAndHumidity();

  datosSensor.temperatura = tempC;
  datosSensor.temperaturaA = data.temperature;
  datosSensor.humedadA = data.humidity;
  datosSensor.humedad = humedads;

  ackRecibido = false;
  esp_now_send(broadcastAddress, (uint8_t*)&datosSensor, sizeof(datosSensor));

  // Esperar ACK hasta 500 ms
  unsigned long t0 = millis();
  while (!ackRecibido && millis() - t0 < 500) {
    delay(10);
  }


  delay(1000);
}
