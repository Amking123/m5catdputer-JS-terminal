#include <M5Cardputer.h>
#include "keyboard.h"
#include "terminalcommands.h"
#include "duktape.h"

duk_context *ctx;

void setup() {
  M5Cardputer.begin();
  M5Cardputer.Display.setRotation(1);
  M5Cardputer.Display.setTextSize(1);
  M5Cardputer.Display.fillScreen(BLACK);

  if (!SD.begin()) {
    shellPrint("SD card init failed!", RED);
    while (1);
  }

  ctx = duk_create_heap_default();
  if (!ctx) {
    shellPrint("Failed to init Duktape!", RED);
    while (1);
  }

  shellPrint("Type 'help' for commands.");
  registerJSBindings(ctx);
  shellDrawPrompt();
}

void loop() {
  if (shellHandleKeyboard()) {
    processCommand(ctx, shellInput);
    shellInput = "";
    shellUpdate = true;
    shellDrawPrompt();
  }

  if (shellUpdate) {
    shellDrawPrompt();
    shellUpdate = false;
  }

  delay(10);
}