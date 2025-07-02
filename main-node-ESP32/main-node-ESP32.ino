#include <SPI.h>
#include <LoRa.h>
#include <esp_now.h>
#include <WiFi.h>

// Configuración de pines ESP32-WROOM-32
#define NSS 5
#define RESET 2
#define SCK 18
#define MISO 19
#define MOSI 23

// Direcciones LoRa
#define LOCAL_ADDRESS 0xAA
#define RECEIVER_ADDRESS 0xFF

int msgCount = 0;

// Estructura del mensaje de datos
typedef struct struct_message {
  float temperatura;
  float temperaturaA;
  float humedadA;
  int humedad;
  uint8_t idNodo;
} struct_message;

struct_message datosRecibidos;

// Estructura ACK
typedef struct struct_ack {
  uint8_t idNodo;
  bool recibido;
} struct_ack;

struct_ack ackMsg;

// Enviar mensaje por LoRa
void sendMessage(String outgoing) {
  LoRa.beginPacket();
  LoRa.write(RECEIVER_ADDRESS);
  LoRa.write(LOCAL_ADDRESS);
  LoRa.write(msgCount);
  LoRa.write(outgoing.length());
  LoRa.print(outgoing);
  LoRa.endPacket();
  msgCount++;
}

// Callback de recepción ESP-NOW
void OnDataRecv(const esp_now_recv_info_t *recv_info, const uint8_t *incomingData, int len) {
  memcpy(&datosRecibidos, incomingData, sizeof(datosRecibidos));

  String mensaje = String(datosRecibidos.idNodo) + "," +
                   String(datosRecibidos.temperatura) + "," +
                   String(datosRecibidos.humedad) + "," +
                   String(datosRecibidos.temperaturaA) + "," +
                   String(datosRecibidos.humedadA);

  sendMessage(mensaje);

  Serial.print("Enviando: ");
  Serial.println(mensaje);

  // Preparar ACK
  ackMsg.idNodo = datosRecibidos.idNodo;
  ackMsg.recibido = true;

  // Enviar ACK a la MAC que envió el paquete
  esp_now_send(recv_info->src_addr, (uint8_t *)&ackMsg, sizeof(ackMsg));
}

void setup() {
  Serial.begin(9600);

  // Configurar SPI con pines específicos
  SPI.begin(SCK, MISO, MOSI, NSS);
  LoRa.setPins(NSS, RESET);

  if (!LoRa.begin(915E6)) {
    Serial.println("Error al iniciar LoRa");
    while (1);
  }

  LoRa.setTxPower(20);
  Serial.println("Transmisor iniciado");

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
  // Vacío: toda la lógica está en el callback OnDataRecv()
}
