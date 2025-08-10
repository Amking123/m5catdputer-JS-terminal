#include <M5Cardputer.h>
#include "keyboard.h"
#include "terminalcommands.h"
#include "duktape.h"
#include <SPI.h>
#include "sharescreen_spi.h"

duk_context *ctx;

// The following are global variables required for the terminal and new functionality.

// Declared in terminalcommands.h, but we need the index here to use it.
extern duk_idx_t onKeyPress_cb_idx;

// The one and only place where the 'shareScreenModeActive' flag is defined.
bool shareScreenModeActive = false;

// The one and only place where the 'hspi' object is defined.
SPIClass * hspi = NULL;

void handleKeyboardEvents(duk_context *ctx) {
    if (onKeyPress_cb_idx == DUK_INVALID_INDEX) {
        return;
    }

    M5Cardputer.update();
    auto ks = M5Cardputer.Keyboard.keysState();

    if (ks.word.empty()) {
        return;
    }

    duk_push_global_object(ctx);
    duk_get_prop_index(ctx, -1, onKeyPress_cb_idx);

    if (duk_is_function(ctx, -1)) {
        duk_push_string(ctx, String(ks.word.back()).c_str());
        duk_pcall(ctx, 1);
    }

    duk_pop_2(ctx);
}

void setup() {
  // CRITICAL FIX: Initialize Serial communication for debugging
  Serial.begin(115200);
  delay(1000); // Give the Serial Monitor time to connect

  Serial.println("--- M5Cardputer Terminal Booting ---");

  M5Cardputer.begin();
  M5Cardputer.Display.setRotation(1);
  M5Cardputer.Display.setTextSize(1);
  M5Cardputer.Display.fillScreen(BLACK);

  Serial.println("M5Cardputer display initialized.");

  // Initialize the SPI master with the new, safe pinout.
  setupMasterSPI();

  if (!SD.begin()) {
    shellPrint("SD card init failed!", RED);
    while (1);
  }

  Serial.println("SD card initialized.");

  ctx = duk_create_heap_default();
  if (!ctx) {
    shellPrint("Failed to init Duktape!", RED);
    while (1);
  }

  shellPrint("Type 'help' for commands.");
  registerJSBindings(ctx);
  shellDrawPrompt();

  Serial.println("Setup complete. Terminal ready.");
}

void loop() {
  handleKeyboardEvents(ctx);

  if (shellHandleKeyboard()) {
    Serial.print("Command received: ");
    Serial.println(shellInput);
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