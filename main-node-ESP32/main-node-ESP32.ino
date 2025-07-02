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
#define LOCAL_ADDRESS 0xAA    // Dirección del transmisor (ESP32)
#define RECEIVER_ADDRESS 0xFF // Dirección del receptor

int msgCount = 0;

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
}

void setup() {
  Serial.begin(115200);
  // Configurar SPI con pines específicos
  SPI.begin(SCK, MISO, MOSI, NSS);
  // Configurar LoRa
  LoRa.setPins(NSS, RESET);

  if (!LoRa.begin(915E6)) {
    Serial.println("Error al iniciar LoRa");
    while (1);
  }
  
  LoRa.setTxPower(20);  // Máxima potencia (20 dBm)
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
void sendMessage(String outgoing) {
  LoRa.beginPacket();
  LoRa.write(RECEIVER_ADDRESS);  // Dirección destino
  LoRa.write(LOCAL_ADDRESS);     // Dirección origen
  LoRa.write(msgCount);          // Contador de mensajes
  LoRa.write(outgoing.length()); // Longitud del mensaje
  LoRa.print(outgoing);          // Contenido del mensaje
  LoRa.endPacket();
  
  msgCount++;
}
void loop() {

  
  String mensaje = String(datosRecibidos.idNodo) + "," + String(datosRecibidos.temperatura) + "," 
  + String(datosRecibidos.humedad) + "," + String(datosRecibidos.temperaturaA) + "," + String(datosRecibidos.humedadA);
  
  // Enviar mensaje
  sendMessage(mensaje);
  
  Serial.print("Enviando: ");
  Serial.println(mensaje);
  
  delay(1000); // Esperar 1 segundos entre transmisiones
}