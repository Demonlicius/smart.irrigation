#include <SPI.h>
#include <LoRa.h>
#include <esp_now.h>
#include <WiFi.h>

// Configuración de pines LoRa
#define NSS 5
#define RESET 2
#define SCK 18
#define MISO 19
#define MOSI 23

// Direcciones LoRa
#define LOCAL_ADDRESS 0xAA
#define RECEIVER_ADDRESS 0xFF

int msgCount = 0;

// Estructura de ACK
typedef struct {
  uint8_t idNodo;
  bool recibido;
} struct_ack;

struct_ack ack;

// --- VARIABLES GLOBALES PARA TRANSFERENCIA ISR -> LOOP ---
char bufferDatos[6][32];        // Buffer para guardar los datos entrantes
uint8_t bufferMacEmisor[6];     // Buffer para guardar quién envió
volatile bool nuevosDatos = false; // Bandera volátil (avisa al loop)
// ---------------------------------------------------------

// Función para enviar por LoRa
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

// Función auxiliar para agregar peer dinámicamente antes de responder
void agregarPeer(const uint8_t *mac_addr) {
  if (!esp_now_is_peer_exist(mac_addr)) {
    esp_now_peer_info_t peer = {};
    memcpy(peer.peer_addr, mac_addr, 6);
    peer.channel = 0;
    peer.encrypt = false;
    esp_now_add_peer(&peer);
  }
}

// Callback de Recepción: AHORA ES RÁPIDO Y SEGURO
void onReceive(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  // Verificamos si el tamaño coincide con nuestro arreglo de datos
  if (len == sizeof(bufferDatos)) {
    // Copiamos datos a variables globales
    memcpy(bufferDatos, data, sizeof(bufferDatos));
    memcpy(bufferMacEmisor, info->src_addr, 6);
    
    // Levantamos la bandera para que el loop() trabaje
    nuevosDatos = true;
  }
}

void setup() {
  Serial.begin(9600);

  // Configurar LoRa
  SPI.begin(SCK, MISO, MOSI, NSS);
  LoRa.setPins(NSS, RESET);

  if (!LoRa.begin(915E6)) { // Asegúrate que sea la frecuencia correcta
    Serial.println("Error al iniciar LoRa");
    while (1);
  }
  LoRa.setTxPower(20);
  Serial.println("Puente Iniciado: ESP-NOW <-> LoRa");

  // Configurar WiFi/ESP-NOW
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error al iniciar ESP-NOW");
    return;
  }
  esp_now_register_recv_cb(onReceive);
}

void loop() {
  // Si la bandera está levantada, procesamos los datos
  if (nuevosDatos) {
    nuevosDatos = false; // Bajamos la bandera inmediatamente

    // 1. Procesar datos para LoRa
    String mensaje = "";
    Serial.println("--- Paquete Recibido ESP-NOW ---");
    for (int i = 0; i < 6; i++) {
      mensaje += String(bufferDatos[i]) + "\n";
      // Debug en serial
      Serial.print("Línea "); Serial.print(i); Serial.print(": ");
      Serial.println(bufferDatos[i]);
    }

    // 2. Enviar por LoRa
    sendMessage(mensaje);
    Serial.println(">> Reenviado por LoRa");

    // 3. Procesar ACK para el Sensor
    int idExtraido = -1;
    // Leemos el ID del primer dato (formato esperado: "ID,temp,hum...")
    sscanf(bufferDatos[0], "%d", &idExtraido);

    if (idExtraido >= 0) {
      ack.idNodo = (uint8_t)idExtraido;
      ack.recibido = true;

      // Aseguramos que el sensor esté en la lista de peers para poder responderle
      agregarPeer(bufferMacEmisor);

      // Enviamos el ACK de vuelta
      esp_err_t result = esp_now_send(bufferMacEmisor, (uint8_t *)&ack, sizeof(ack));
      
      if(result == ESP_OK){
        Serial.println("<< ACK enviado al Sensor");
      } else {
        Serial.println("Error enviando ACK");
      }
    }
    Serial.println("-------------------------------");
  }
}