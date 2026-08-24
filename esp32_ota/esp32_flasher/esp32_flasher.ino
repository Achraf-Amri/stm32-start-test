#include <WiFi.h>
#include <WebServer.h>
#include <soc/rtc_cntl_reg.h>
#include <soc/soc.h>
#include "Stm32_usart_bootloader.h"

// --- Configuration Wi-Fi ---
const char* ssid = "SOFIA_R1";
const char* password = "SOFIA-R1-WIFI@2024***";

// --- Configuration des broches GPIO ---
#define BOOT0_PIN 4  // Modifiez si vous utilisez un autre GPIO pour BOOT0
#define NRST_PIN  5  // GPIO5 connecté à NRST

WebServer server(80);
Stm32_usart_bootloader* bl;

uint8_t firmwareBuffer[65536]; // Buffer de 64 Ko
size_t firmwareSize = 0;

void enterBootloaderMode() {
  digitalWrite(BOOT0_PIN, HIGH);
  digitalWrite(NRST_PIN, LOW);
  delay(100);
  digitalWrite(NRST_PIN, HIGH);
  delay(100);
}

void exitBootloaderMode() {
  digitalWrite(BOOT0_PIN, LOW);
  digitalWrite(NRST_PIN, LOW);
  delay(100);
  digitalWrite(NRST_PIN, HIGH);
}

// Effacement rapide du Secteur 0 (16 Ko) pour STM32F4
bool eraseSTM32F4() {
  while (Serial2.available() > 0) Serial2.read(); // Purge buffer

  // 1. Envoi de la commande Extended Erase (0x44) + complément (0xBB)
  Serial2.write(0x44);
  Serial2.write(0xBB);

  unsigned long timeout = millis();
  while (!Serial2.available()) {
    if (millis() - timeout > 2000) return false;
  }
  if (Serial2.read() != 0x79) return false; // Attente ACK (0x79)

  // 2. Effacement du Secteur 0 : [N-1=0 (2 octets)] [Secteur 0 (2 octets)] [Checksum (1 octet)]
  uint8_t sector0Cmd[5] = {0x00, 0x00, 0x00, 0x00, 0x00};
  Serial2.write(sector0Cmd, 5);

  // 3. Attente du succès (ultra-rapide, < 1s)
  timeout = millis();
  while (!Serial2.available()) {
    if (millis() - timeout > 5000) return false;
  }
  return (Serial2.read() == 0x79);
}

void handleFlashUpload() {
  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    firmwareSize = 0;
    Serial.println("Debut upload firmware...");
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (firmwareSize + upload.currentSize <= sizeof(firmwareBuffer)) {
      memcpy(firmwareBuffer + firmwareSize, upload.buf, upload.currentSize);
      firmwareSize += upload.currentSize;
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    Serial.printf("Upload termine : %d octets\n", firmwareSize);
  }
}

void handleFlash() {
  Serial.printf("Firmware recu : %d octets\n", firmwareSize);
  Serial.flush();

  if (firmwareSize == 0) {
    server.send(400, "text/plain", "Taille de firmware invalide");
    return;
  }

  Serial.println("1. Basculement des broches (BOOT0 / NRST)...");
  enterBootloaderMode();
  delay(100);

  while (Serial2.available() > 0) Serial2.read(); // Purge de la liaison série

  Serial.println("2. Envoi de l'octet de synchronisation au STM32...");
  uint8_t enterStatus = bl->enter();

  if (enterStatus != STM32_ACK) {
    Serial.printf("ERREUR : Le STM32 n'a pas repondu (Code: 0x%02X)\n", enterStatus);
    server.send(500, "text/plain", "Echec entree bootloader STM32");
    exitBootloaderMode();
    return;
  }

  Serial.println("2b. Effacement Extended Erase (0x44) de la Flash STM32F4...");
  if (!eraseSTM32F4()) {
    Serial.println("ERREUR : Echec de l'effacement Flash STM32F4");
    server.send(500, "text/plain", "Echec effacement Flash STM32F4");
    exitBootloaderMode();
    return;
  }
  Serial.println("Effacement réussi !");

  Serial.println("3. Synchronisation reussie ! Programmation de la memoire Flash en cours...");
  uint8_t result = bl->program(firmwareBuffer, firmwareSize);

  if (result == STM32_ACK) {
    Serial.println("4. FLASH REUSSI ! Lancement de l'application STM32...");
    bl->go(0x08000000);
    exitBootloaderMode();
    server.send(200, "text/plain", "Flash reussi");
  } else {
    Serial.printf("4. ERREUR : Echec pendant la programmation (Code: 0x%02X)\n", result);
    exitBootloaderMode();
    server.send(500, "text/plain", "Echec ecriture flash");
  }
}

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // Désactive le détecteur de sous-tension
  Serial.begin(115200);
  delay(1000);

  pinMode(BOOT0_PIN, OUTPUT);
  pinMode(NRST_PIN, OUTPUT);
  digitalWrite(NRST_PIN, HIGH);
  digitalWrite(BOOT0_PIN, LOW);

  Serial2.begin(57600, SERIAL_8E1, 16, 17); // RX2=GPIO16, TX2=GPIO17
  bl = new Stm32_usart_bootloader(&Serial2);

  WiFi.begin(ssid, password);
  Serial.print("Connexion WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("IP ESP32 : ");
  Serial.println(WiFi.localIP());

  const char* headerKeys[] = {"Content-Length"};
  server.collectHeaders(headerKeys, 1);
  server.on("/flash", HTTP_POST, handleFlash, handleFlashUpload);
  server.begin();
}

void loop() {
  server.handleClient();
}
