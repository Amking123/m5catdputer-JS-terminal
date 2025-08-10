#ifndef MENU_H
#define MENU_H

#include <M5Cardputer.h>
#include "duktape.h"

// Sample menu items
const char* menuItems[] = { "DOOM_3D", "terminal", "Flappybird" };
const int menuCount = sizeof(menuItems) / sizeof(menuItems[0]);

int selected = 0;

void drawMenu() {
  M5Cardputer.Display.fillScreen(BLACK);
  M5Cardputer.Display.setTextSize(2);

  for (int i = 0; i < menuCount; i++) {
    int y = 30 + i * 30;
    if (i == selected) {
      M5Cardputer.Display.fillRect(0, y - 5, 240, 25, DARKGREY);
    }
    M5Cardputer.Display.setCursor(10, y);
    M5Cardputer.Display.setTextColor(i == selected ? YELLOW : WHITE);
    M5Cardputer.Display.print(menuItems[i]);
  }
}

void openMenu(duk_context* ctx) {
  drawMenu();

  while (true) {
    M5Cardputer.update();
    auto keys = M5Cardputer.Keyboard.keysState();

    // Loop through pressed keys
    for (char k : keys.word) {
      if (k == ';' || k == ':') {
        selected = (selected - 1 + menuCount) % menuCount;
        drawMenu();
        delay(150);
      } else if (k == '.' || k == '>') {
        selected = (selected + 1) % menuCount;
        drawMenu();
        delay(150);
      } else if (k == ' ') {  // Enter key
        // Run JS app based on selected menu item
        String appName = "/apps/" + String(menuItems[selected]) + ".js";
        if (SD.exists(appName)) {
          File f = SD.open(appName);
          if (f) {
            String code = f.readString();
            f.close();
            if (duk_peval_string(ctx, code.c_str()) != 0) {
              M5Cardputer.Display.println("Error:");
              M5Cardputer.Display.println(duk_safe_to_string(ctx, -1));
              duk_pop(ctx);
            } else {
              duk_pop(ctx);
            }
          }
        } else {
          M5Cardputer.Display.println("\aApp not found");
        }
        return;  // Exit menu after running
      } else if (k == '`' || k == '~') {
        return;  // Exit menu
      }
    }

    delay(50);
  }
}
#endif