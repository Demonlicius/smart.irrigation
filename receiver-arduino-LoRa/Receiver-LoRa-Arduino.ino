#include <SPI.h>
#include <LoRa.h>

#define NSS   10  // CS
#define RST    9  // Reset LoRa

#define LOCAL_ADDRESS    0xFF  // Dirección del receptor (Arduino Uno)

int countmsg = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  LoRa.setPins(NSS, RST); // Sin DIO0

  if (!LoRa.begin(915E6)) {
    Serial.println("Fallo al inicializar LoRa");
    while (true);
  }

  Serial.println("Receptor LoRa listo");
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
  
  countmsg = countmsg + 1;

  if (message.length() != length) return;

  Serial.print(countmsg);
  Serial.print(" 0x");
  Serial.print(sender, HEX);
  Serial.print(",");
  Serial.println(message);
}
