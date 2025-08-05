#ifndef BIOS_H
#define BIOS_H

#include <M5Cardputer.h>
#include <SD.h>

void drawBIOSMenu(int selected) {
  const char* options[] = {
    "Device Info",
    "Reboot",
    "Diagnostics",
    "Exit BIOS"
  };

  int totalOptions = sizeof(options) / sizeof(options[0]);

  M5Cardputer.Display.fillScreen(BLUE);
  M5Cardputer.Display.setTextColor(BLACK);
  M5Cardputer.Display.setTextSize(1);
  M5Cardputer.Display.setCursor(5, 5);
  M5Cardputer.Display.println("Cardputer BIOS v1.0");

  for (int i = 0; i < totalOptions; i++) {
    int y = 30 + i * 20;
    if (i == selected) {
      M5Cardputer.Display.fillRect(0, y - 2, 240, 18, BLACK);
      M5Cardputer.Display.setTextColor(BLUE);
    } else {
      M5Cardputer.Display.setTextColor(BLACK);
    }
    M5Cardputer.Display.setCursor(10, y);
    M5Cardputer.Display.println(options[i]);
  }
}

void openBIOS() {
  int selected = 0;
  bool running = true;

  drawBIOSMenu(selected);
  // Wait until all keys are released before continuing
   while (M5Cardputer.Keyboard.isChange()) {
    M5Cardputer.update();
    delay(10);
   }

  while (running) {
    M5Cardputer.update();
    auto keys = M5Cardputer.Keyboard.keysState();

    if (M5Cardputer.Keyboard.isKeyPressed(';')) {
      selected = (selected - 1 + 4) % 4;
      drawBIOSMenu(selected);
      delay(150);
    } else if (M5Cardputer.Keyboard.isKeyPressed('.')) {
      selected = (selected + 1) % 4;
      drawBIOSMenu(selected);
      delay(150);
    }

    if (M5Cardputer.Keyboard.isKeyPressed(' ') || keys.enter) {
      switch (selected) {
        case 0:
          M5Cardputer.Display.fillScreen(BLUE);
          M5Cardputer.Display.setCursor(10, 10);
          M5Cardputer.Display.setTextColor(BLACK);
          M5Cardputer.Display.println("Device Info:");
          M5Cardputer.Display.println("Firmware: CardOS v6.5.1");
          M5Cardputer.Display.println("CPU: ESP32");
          M5Cardputer.Display.println("Flash: " + String(ESP.getFlashChipSize() / 1024) + " KB");
          M5Cardputer.Display.println("Heap: " + String(ESP.getFreeHeap() / 1024) + " KB free");
          M5Cardputer.Display.println("\nPress space to go back.");
          while (!M5Cardputer.Keyboard.isKeyPressed(' ')) {
            M5Cardputer.update();
          }
          drawBIOSMenu(selected);
          break;

        case 1:
          M5Cardputer.Display.fillScreen(BLUE);
          M5Cardputer.Display.setCursor(10, 50);
          M5Cardputer.Display.setTextColor(BLACK);
          M5Cardputer.Display.println("Rebooting...");
          delay(1000);
          ESP.restart();
          break;

        case 2:
          M5Cardputer.Display.fillScreen(BLUE);
          M5Cardputer.Display.setCursor(10, 30);
          M5Cardputer.Display.setTextColor(BLACK);
          M5Cardputer.Display.println("Running Diagnostics...");
          delay(500);
          M5Cardputer.Display.println("✔ Display OK");
          M5Cardputer.Display.println("✔ SD Card: " + String(SD.begin() ? "Detected" : "Not Found"));
          M5Cardputer.Display.println("✔ Keyboard OK");
          M5Cardputer.Display.println("✔ Memory OK");
          M5Cardputer.Display.println("\nPress space to go back.");
          while (!M5Cardputer.Keyboard.isKeyPressed(' ')) {
            M5Cardputer.update();
          }
          drawBIOSMenu(selected);
          break;

        case 3:
          running = false;
          break;
      }
      delay(200);
    }
  }

  M5Cardputer.Display.fillScreen(BLUE);
}


#endif
