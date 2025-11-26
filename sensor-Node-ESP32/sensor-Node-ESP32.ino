#include <esp_now.h>
#include <WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "DHTesp.h"

#define ONE_WIRE_BUS 27
#define sensor 32
int pinDHT = 25;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DHTesp dht;

// Dirección MAC del receptor
uint8_t receptorMAC[] = {0x78, 0x42, 0x1C, 0x67, 0xAA, 0x2C}; // Ajusta según tu nodo receptor

// Mensaje a enviar: arreglo de 6 líneas de texto
char datos[6][32];
unsigned long t0 = 0;
const unsigned long intervalo = 10000;  // 10 segundos
int lineaindex = 0;

// ACK de aplicación
typedef struct {
  uint8_t idNodo;
  bool recibido;
} struct_ack;

volatile bool ackRecibido = false;
struct_ack ack;
uint8_t idNodo = 5;

// Callback recepción ACK
void onReceive(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  if (len == sizeof(struct_ack)) {
    memcpy(&ack, data, sizeof(ack));
    if (ack.recibido && ack.idNodo == idNodo) {
      ackRecibido = true;
    }
  }
}

// Callback para confirmar si se recibió ACK
void onSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Estado de envio: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "ACK recibido" : "Fallo");
}

void setup() {
  Serial.begin(9600);
  
  sensors.begin();
  dht.setup(pinDHT, DHTesp::DHT11);

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error iniciando ESP-NOW");
    return;
  }

  esp_now_register_send_cb(onSent);
  esp_now_register_recv_cb(onReceive);

  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, receptorMAC, 6);
  peer.channel = 0;
  peer.encrypt = false;

  if (!esp_now_is_peer_exist(receptorMAC)) {
    esp_now_add_peer(&peer);
  }
}

void loop() {

  if(millis() - t0 >= intervalo){
    t0 = millis();

    sensors.requestTemperatures(); // Send the command to get temperatures
    TempAndHumidity data = dht.getTempAndHumidity();
    float temperatura_soil = sensors.getTempCByIndex(0);
    int   humedad_suelo = map(analogRead(sensor),1200,4095,100,0);
    float humedad_ambiental = data.humidity;
    float temperatura_ambiental = data.temperature;

    snprintf(datos[lineaindex], sizeof(datos[lineaindex]), "%d,%.2f,%d,%.2f,%.2f",idNodo,temperatura_soil,humedad_suelo
    ,humedad_ambiental,temperatura_ambiental);

    lineaindex++;

    if (lineaindex>=6) {
      lineaindex = 0;

      esp_err_t result = esp_now_send(receptorMAC, (uint8_t *)datos, sizeof(datos));
      if (result != ESP_OK) {
        Serial.println("Error al enviar");
      } else {
        unsigned long espera = millis();
        while (!ackRecibido && millis() - espera < 500){
          delay(10);
        }
      }
    }
  }
}
