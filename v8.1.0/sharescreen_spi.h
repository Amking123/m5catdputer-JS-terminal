#ifndef SHARESCREEN_SPI_H
#define SHARESCREEN_SPI_H

#include <SPI.h>

#define SPI_SCK_PIN 1
#define SPI_MOSI_PIN 48
#define SPI_MISO_PIN 21
#define SPI_CS_PIN 2

extern SPIClass * hspi;
extern bool shareScreenModeActive;

typedef struct {
  char command[32];
  char data[220];
} ShareScreenData;

void setupMasterSPI() {
  hspi = new SPIClass(HSPI);
  hspi->begin(SPI_SCK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN, -1);
  pinMode(SPI_CS_PIN, OUTPUT);
  digitalWrite(SPI_CS_PIN, HIGH);
}

void sendShareScreenCommand(const char* command, const char* data = "") {
  ShareScreenData myData;
  strncpy(myData.command, command, sizeof(myData.command) - 1);
  strncpy(myData.data, data, sizeof(myData.data) - 1);
  Serial.print("Sending SPI command: ");
  Serial.print(command);
  Serial.print(" with data: ");
  Serial.println(data);
  digitalWrite(SPI_CS_PIN, LOW);
  hspi->beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0));
  hspi->transfer((uint8_t*)&myData, sizeof(myData));
  hspi->endTransaction();
  digitalWrite(SPI_CS_PIN, HIGH);
}

void sendShareScreenText(const char* text) {
  sendShareScreenCommand("TEXT", text);
}

void sendShareScreenClear() {
  sendShareScreenCommand("CLEAR");
}
#endif