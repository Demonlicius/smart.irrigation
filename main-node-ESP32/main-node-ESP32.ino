#include <esp_now.h>
#include <WiFi.h>

typedef struct struct_message {
  float temperatura;
  float temperaturaA;
  float humedadA;
  int humedad;

  uint8_t idNodo;
} struct_message;

struct_message datosRecibidos;

void OnDataRecv(const esp_now_recv_info_t *recv_info, const uint8_t *incomingData, int len) {
  memcpy(&datosRecibidos, incomingData, sizeof(datosRecibidos));

  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           recv_info->src_addr[0], recv_info->src_addr[1], recv_info->src_addr[2],
           recv_info->src_addr[3], recv_info->src_addr[4], recv_info->src_addr[5]);
           
  Serial.print("Nodo ID: ");
  Serial.print(datosRecibidos.idNodo);
  Serial.println("");
  Serial.print("Temperatura: ");
  Serial.print(datosRecibidos.temperatura);
  Serial.print("°C | Humedad: ");
  Serial.print(datosRecibidos.humedad);
  Serial.println("%");
    Serial.print("Temperatura ambiental: ");
  Serial.print(datosRecibidos.temperaturaA);
  Serial.print("°C | Humedad ambiental: ");
  Serial.print(datosRecibidos.humedadA);
  Serial.println("%");
}

void setup() {
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);
  Serial.print("MAC del nodo principal: ");
  Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error al iniciar ESP-NOW");
    return;
  }

  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  
}