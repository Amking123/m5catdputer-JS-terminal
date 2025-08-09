#include <M5Cardputer.h>
#include "keyboard.h"
#include "terminalcommands.h" // The updated header file
#include "duktape.h"

duk_context *ctx;

// Declare the global variable from terminalcommands.h so it can be used here.
extern duk_idx_t onKeyPress_cb_idx;

// A new function to check for keyboard events and fire the JS callback.
// This function needs to be in your main .ino file to have access to the ctx.
void handleKeyboardEvents(duk_context *ctx) {
    // If no JavaScript callback function has been registered, do nothing.
    if (onKeyPress_cb_idx == DUK_INVALID_INDEX) {
        return;
    }

    // Update the keyboard state to get the latest key presses.
    M5Cardputer.update();
    auto ks = M5Cardputer.Keyboard.keysState();

    // If there's no key currently pressed, we're done.
    if (ks.word.empty()) {
        return;
    }

    // A key was pressed. Get the callback function from the Duktape stack.
    duk_push_global_object(ctx);
    duk_get_prop_index(ctx, -1, onKeyPress_cb_idx);
    
    // Check if the item we retrieved is a function.
    if (duk_is_function(ctx, -1)) {
        // Push the key as a string argument for the JavaScript function.
        duk_push_string(ctx, String(ks.word.back()).c_str());
        
        // Call the JavaScript function with one argument.
        duk_pcall(ctx, 1);
    }
    
    // Pop the global object and the function from the Duktape stack.
    duk_pop_2(ctx);
}

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
  // NEW: Handle event-driven JS keyboard events first.
  // This will check for key presses continuously and call any registered
  // JavaScript callback function. This is the core of your UI's interactivity.
  handleKeyboardEvents(ctx);

  // Existing terminal logic.
  // This will only run and process a command when the Enter key is pressed.
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

  // The delay is fine for a terminal, but for a fast-responding UI, you may
  // want to reduce or remove it in your final UI application.
  delay(10); 
}
