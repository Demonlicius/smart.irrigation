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

char datos[6][32];

// ACK a nivel aplicación
typedef struct {
  uint8_t idNodo;
  bool recibido;
} struct_ack;

struct_ack ack;

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

void onReceive(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  if (len == sizeof(datos)) {
    memcpy(datos, data, sizeof(datos));
    String mensaje = "";
    Serial.println("Datos recibidos:");
    for (int i = 0; i < 6; i++) {
      Serial.print("Línea ");
      Serial.print(i);
      Serial.print(": ");
      Serial.println(datos[i]);
      mensaje += String(datos[i]) + "\n";
    }

    sendMessage(mensaje);

    int idExtraido = -1;
    sscanf(datos[0], "%d", &idExtraido);
    if (idExtraido >= 0) {
      ack.idNodo = (uint8_t)idExtraido;
      ack.recibido = true;
      esp_now_send(info->src_addr, (uint8_t *)&ack, sizeof(ack));
    }
  }
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

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error al iniciar ESP-NOW");
    return;
  }

  esp_now_register_recv_cb(onReceive);
}

void loop() {
  // vacío
}
