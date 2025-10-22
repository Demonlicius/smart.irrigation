#include <SPI.h>
#include <LoRa.h>
#include <esp_now.h>
#include <WiFi.h>

#define NSS 5
#define RESET 2
#define SCK 18
#define MISO 19
#define MOSI 23

#define LOCAL_ADDRESS    0xFF  // Direccion del receptor

int countmsg = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  SPI.begin(SCK, MISO, MOSI, NSS);
  LoRa.setPins(NSS, RESET); // Sin DIO0

  if (!LoRa.begin(915E6)) {
    Serial.println("Fallo al inicializar LoRa");
    while (true);
  }

  
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize == 0) return;

  byte recipient = LoRa.read();
  if (recipient != LOCAL_ADDRESS && recipient != 0xFF) return;

  byte sender = LoRa.read();
  byte msgId = LoRa.read();
  byte length = LoRa.read();

  String message = "";
  while (LoRa.available()) {
    message += (char)LoRa.read();
  }
  
  if (message.length() != length) return;
  Serial.print(sender, HEX);
  Serial.print(",");
  Serial.println(message);
}
